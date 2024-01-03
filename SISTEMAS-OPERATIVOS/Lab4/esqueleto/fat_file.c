/*
 * fat_file.c
 *
 * Functions to operate over files and directories.
 */

#include <alloca.h>
#include <errno.h>
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "fat_file.h"
#include "fat_filename_util.h"
#include "fat_table.h"
#include "fat_util.h"

static void write_dir_entry(fat_file parent, fat_dir_entry child_disk_entry,
                            u32 nentry);

/* Fills fields last_modified_time, last_modified_date, last_access_date,
 * create_date and create_time of a *in disk* file directory entry @dir_entry,
 * with the current time.
 * If @fill_creation or @fill_modified are false, it does not fill the
 * corresponding fields.
 * PRE: @fill_create => @fill_modified
 */
static int fill_dentry_time_now(fat_dir_entry dir_entry, bool fill_create,
                                bool fill_modified) {
    int ret = 0;
    le16 entry_date = 0, entry_time = 0;
    time_t now = time(NULL);

    if (!(!fill_create || fill_modified))
        return -1;
    if (fill_modified) {
        ret = fill_time(&entry_date, &entry_time, now);
        dir_entry->last_modified_date = entry_date;
        dir_entry->last_modified_time = entry_time;
        dir_entry->last_access_date = entry_date;
    } else {
        ret = fill_time(&entry_date, NULL, now);
        dir_entry->last_access_date = entry_date;
    }
    if (ret != 0)
        return ret;
    if (fill_create) {
        dir_entry->create_date = dir_entry->last_modified_date;
        dir_entry->create_time = dir_entry->last_modified_time;
    }
    return ret;
}

/* Set the information for first cluster in the directory entry @dir_entry.
 * Copied from libfat. */
static int set_first_cluster(fat_dir_entry dir_entry, u32 cluster) {
    if (dir_entry == NULL)
        return -1;
    cluster = cpu_to_le32(cluster); // cluster is now little endian

    char *src = (char *)&cluster;
    char *dst = (char *)&(dir_entry->start_cluster_low);

    dst[0] = src[0];
    dst[1] = src[1];

    dst = (char *)&(dir_entry->fat32_start_cluster_high);
    dst[0] = src[2];
    dst[1] = src[3];

    return 0;
}

/* Returns the start cluster from the @disk_entry */
static u32 file_start_cluster(fat_dir_entry disk_dentry) {
    u32 start_cluster;
    start_cluster = le16_to_cpu(disk_dentry->start_cluster_low);

    // FAT32 uses a 32-bit start cluster field, but it's split into
    // two different 16-bit locations on disk.
    start_cluster |= (u32)le16_to_cpu(disk_dentry->fat32_start_cluster_high)
                     << 16;
    return start_cluster;
}

/********************* INITIALIZERS *********************/

/* Returns an new directory entry with default values.
 * Values to fill depending on fat_file: start_cluster
 */
fat_dir_entry fat_file_init_direntry(bool is_dir, char *filepath,
                                     u32 start_cluster) {
    fat_dir_entry new_entry = calloc(1, sizeof(struct fat_dir_entry_s));
    if (new_entry == NULL) {
        errno = ENOSPC;
        return NULL;
    }
    // Calculate filename and extension. Save into disk entry structure
    filename_from_path(basename(strdup(filepath)), new_entry->base_name,
                       new_entry->extension);
    if (is_dir) {
        new_entry->attribs = FILE_ATTRIBUTE_DIRECTORY;
    } else {
        new_entry->attribs = FILE_ATTRIBUTE_ARCHIVE;
    }
    new_entry->reserved = 0;
    new_entry->create_time_fine_res = 0;
    fill_dentry_time_now(new_entry, true, true); // ignore error
    set_first_cluster(new_entry, start_cluster);
    new_entry->file_size = 0;
    return new_entry;
}

/* Returns an new directory entry with default values.
 * Values to fill depending on fat_file: start_cluster, pos_in_dir
 */
static fat_dir_entry init_direntry_from_buff(fat_dir_entry buff) {
    fat_dir_entry new_entry = calloc(1, sizeof(struct fat_dir_entry_s));
    if (new_entry == NULL) {
        errno = ENOSPC;
        return NULL;
    }
    memcpy(new_entry, buff, sizeof(struct fat_dir_entry_s));

    return new_entry;
}

/* Creates a fat_file from the information contained in @dentry.
 * @parent can't be None, since root directory does not have a dentry.*/
static fat_file init_file_from_dentry(fat_dir_entry dentry, fat_file parent) {
    fat_file new_file = NULL;
    bool is_dir = (dentry->attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;

    new_file = calloc(1, sizeof(struct fat_file_s));
    if (new_file == NULL) {
        errno = ENOSPC;
        return NULL;
    }
    new_file->start_cluster = file_start_cluster(dentry);
    new_file->dentry = dentry;
    new_file->table = parent->table;
    build_filename(dentry->base_name, dentry->extension,
                   (char *)&(new_file->name));
    new_file->filepath = filepath_from_name(parent->filepath, new_file->name);
    if (is_dir) {
        new_file->dir.nentries = 0;
    } else {
        new_file->file.num_clusters = 0;
    }
    new_file->pos_in_parent = parent->dir.nentries;
    new_file->num_times_opened = 0;
    new_file->children_read = 0;
    return new_file;
}

fat_file fat_file_init_empty(bool is_dir, char *filepath) {
    fat_file new_file = NULL;
    new_file = calloc(1, sizeof(struct fat_file_s));
    if (new_file == NULL) {
        errno = ENOSPC;
        return NULL;
    }
    new_file->filepath = filepath;
    if (is_dir) {
        new_file->dir.nentries = 0;
    } else {
        new_file->file.num_clusters = 0;
    }
    new_file->pos_in_parent = 0;
    new_file->num_times_opened = 0;
    new_file->children_read = 0;
    new_file->dentry = NULL;
    return new_file;
}

fat_file fat_file_init(fat_table table, bool is_dir, char *filepath) {
    fat_file new_file = fat_file_init_empty(is_dir, filepath);
    fat_dir_entry new_entry = NULL;
    if (errno < 0) {
        free(new_file);
        free(new_entry);
        return NULL;
    }
    new_file->table = table;

    u32 start_cluster = fat_table_get_next_free_cluster(table);
    if (fat_table_is_cluster_used(table, start_cluster)) {
        DEBUG("Assigned cluster in use!");
        errno = ENOSPC;
        fat_file_destroy(new_file);
        return NULL;
    }
    new_file->dentry = fat_file_init_direntry(is_dir, filepath, start_cluster);

    build_filename(new_file->dentry->base_name, new_file->dentry->extension,
                   (char *)&(new_file->name));
    new_file->start_cluster = file_start_cluster(new_file->dentry);

    fat_table_set_next_cluster(table, start_cluster, FAT_CLUSTER_END_OF_CHAIN);
    if (errno != 0) {
        fat_file_destroy(new_file);
        return NULL;
    }
    return new_file;
}

/* Frees memory allocated for the fat_file_s structure. */
void fat_file_destroy(fat_file file) {
    free(file->filepath);
    free(file->dentry);
    free(file);
}

int fat_file_cmp(fat_file file1, fat_file file2) {
    return strcmp(file1->filepath, file2->filepath);
}

int fat_file_cmp_path(fat_file file1, char *filepath) {
    return strcmp(file1->filepath, filepath);
}

/********************* FILE METADATA *********************/

inline bool fat_file_is_directory(const fat_file file) {
    if (file->dentry == NULL) {
        return true; // root file
    }
    return (file->dentry->attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void fat_file_inc_num_times_opened(fat_file file) {
    if (file != NULL) {
        file->num_times_opened++;
    }
}

void fat_file_dec_num_times_opened(fat_file file) {
    if (file != NULL) {
        file->num_times_opened--;
    }
}

void fat_file_to_stbuf(fat_file file, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(*stbuf));
    stbuf->st_nlink = 1;
    if (fat_file_is_directory(file)) {
        stbuf->st_mode |= S_IFDIR;
    } else {
        stbuf->st_mode |= S_IFREG;
    }
    if (file->dentry->attribs & FILE_ATTRIBUTE_READONLY) {
        stbuf->st_mode |= 0555;
    } else {
        stbuf->st_mode |= 0777;
    }
    stbuf->st_size = file->dentry->file_size;
    stbuf->st_blocks =
        fat_table_get_clusters_for_size(file->table, stbuf->st_size);
    stbuf->st_blksize = fat_table_bytes_per_cluster(file->table);
    stbuf->st_ctime = time_to_unix_time(file->dentry->create_date, // 0);
                                        file->dentry->create_time);
    stbuf->st_atime = time_to_unix_time(file->dentry->last_access_date, 0);
    stbuf->st_mtime = time_to_unix_time(file->dentry->last_modified_date, // 0);
                                        file->dentry->last_modified_time);
}

/********************* DIRECTORY ENTRY METADATA *********************/

/* Prints dentry information. Modify to suit your use case */
void fat_file_print_dentry(fat_dir_entry dentry) {
    printf("\t Writting dentry: \n");
    printf("\t Basename: %.*s\n", 8, dentry->base_name);
    printf("\t Extension: %.*s\n", 3, dentry->extension);
    printf("\t Attributes: %x\n", dentry->attribs);
}

/* Writes to disk @child_disk_entry, in the position @nentry of the @parent*/
static void write_dir_entry(fat_file parent, fat_dir_entry child_disk_entry,
                            u32 nentry) {
    // Calculate the starting position of the directory
    u32 chunk_size = fat_table_bytes_per_cluster(parent->table);
    off_t parent_offset =
        fat_table_cluster_offset(parent->table, parent->start_cluster);
    size_t entry_size = sizeof(struct fat_dir_entry_s);
    if (chunk_size <= nentry * entry_size) {
        errno = ENOSPC; // TODO we should add a new cluster to the directory.
        DEBUG("The parent directory is full.");
        return;
    }
    DEBUG("Writting dentry on directory %s, entry %u", parent->name, nentry);
    // Calculate the position of the next entry
    off_t entry_offset = (off_t)(nentry * entry_size) + parent_offset;
    ssize_t written_bytes =
        pwrite(parent->table->fd, child_disk_entry, entry_size, entry_offset);
    if (written_bytes < entry_size) {
        errno = EIO;
        DEBUG("Error writing child disk entry");
    }
}

void fat_utime(fat_file file, fat_file parent, const struct utimbuf *buf) {
    // Adapted from libfat
    le16 accdate, acctime, moddate, modtime;
    // All in little endians!
    fill_time(&accdate, &acctime, buf->actime);
    fill_time(&moddate, &modtime, buf->modtime);
    file->dentry->last_access_date = accdate;
    file->dentry->last_modified_date = moddate;
    file->dentry->last_modified_time = modtime;
    write_dir_entry(parent, file->dentry, file->pos_in_parent);
}

/********************* DIRECTORY FUNCTIONS *********************/

void fat_file_dentry_add_child(fat_file parent, fat_file child) {
    u32 nentries = parent->dir.nentries;
    write_dir_entry(parent, child->dentry, nentries);
    if (errno != 0) {
        return;
    }
    DEBUG("Adding child \"%s\" to \"%s\" in position %u", child->name,
          parent->filepath, parent->dir.nentries);
    child->pos_in_parent = nentries;
    parent->dir.nentries++;
}

/* Returns %true iff the given FAT on-disk directory entry is the special
 * end-of-directory entry. */
static bool is_end_of_directory(const fat_dir_entry disk_dentry) {
    return disk_dentry->base_name[0] == '\0';
}

/* Returns %true iff the filesystem driver should ignore the given directory
 * entry due to having invalid attributes or an invalid name. */
static bool ignore_dentry(const fat_dir_entry disk_dentry) {
    // Note: VFAT entries have FILE_ATTRIBUTE_VOLUME set, so they will be
    // correctly ignored by this long-name unaware code.
    return (disk_dentry->attribs & (FILE_ATTRIBUTE_VOLUME)) ||
           !file_basename_valid(disk_dentry->base_name) ||
           !file_extension_valid(disk_dentry->extension);
}

/* Fills @elems with the fat_dir_entry that's read form @buffer, and
 * updates @dir to mark the children have been read. @end_ptr is used
 * to mark the end of the @buffer.
 * @dir can't be NULL, since root directory does not have a dentry.
 */
static void read_cluster_dir_entries(u8 *buffer, fat_dir_entry end_ptr,
                                     fat_file dir, GList **elems) {
    fat_dir_entry disk_dentry_ptr = NULL;
    u32 dir_entries_processed = 0;
    char is_log;

    for (disk_dentry_ptr = (fat_dir_entry)buffer; disk_dentry_ptr <= end_ptr;
         disk_dentry_ptr++, dir_entries_processed++) {
        is_log = 0;

        if (is_end_of_directory(disk_dentry_ptr)) {
            dir->children_read = 1;
            break;
        }
        if (ignore_dentry(disk_dentry_ptr)) { // fs.log      (0xe5)s ->
                                              // base_name      log -> extension
            if (!((char)disk_dentry_ptr->base_name[0] == (char)0xe5 &&
                  disk_dentry_ptr->base_name[1] == 's' &&
                  ((char *)disk_dentry_ptr->extension)[0] == 'l' &&
                  ((char *)disk_dentry_ptr->extension)[1] == 'o' &&
                  ((char *)disk_dentry_ptr->extension)[2] == 'g')) {
                continue; // PROCEDA!
            } else {
                is_log = 1;
            }
        }
        // Create and fill new child structure
        fat_dir_entry new_entry = init_direntry_from_buff(disk_dentry_ptr);
        fat_file child = init_file_from_dentry(new_entry, dir);

        if (is_log) {
            child->name[0] = 'f';
            child->filepath[1] = 'f'; //-----> "/fs.log"
        }
        (*elems) = g_list_append((*elems), child);
    }
    dir->dir.nentries = dir_entries_processed;
}

GList *fat_file_read_children(fat_file dir) {
    u32 bytes_per_cluster = 0, cur_cluster = 0;
    off_t cur_offset = 0;
    u8 *buf = NULL;
    GList *entry_list = NULL;

    DEBUG("Reading children of \"%s\"", dir->filepath);
    bytes_per_cluster = fat_table_bytes_per_cluster(dir->table);
    cur_cluster = dir->start_cluster;
    if (!fat_table_is_valid_cluster_number(dir->table, cur_cluster)) {
        fat_error("Cluster number %u is invalid", cur_cluster);
        errno = EIO;
        return NULL;
    }
    cur_offset = fat_table_cluster_offset(dir->table, cur_cluster);

    buf = alloca(bytes_per_cluster);
    while (!fat_table_is_EOC(dir->table, cur_cluster)) {
        fat_dir_entry end_ptr;
        end_ptr = (fat_dir_entry)(buf + bytes_per_cluster) - 1;
        if (full_pread(dir->table->fd, buf, bytes_per_cluster, cur_offset) !=
            bytes_per_cluster) {
            errno = EIO;
            return NULL;
        }
        read_cluster_dir_entries(buf, end_ptr, dir, &entry_list);
        cur_cluster = fat_table_get_next_cluster(dir->table, cur_cluster);
        cur_offset = fat_table_cluster_offset(dir->table, cur_cluster);
    }
    dir->children_read = 1;
    return entry_list;
}

/********************* READ/WRITE OPERATIONS *********************/

ssize_t fat_file_pread(fat_file file, void *buf, size_t size, off_t offset,
                       fat_file parent) {
    if (offset > file->dentry->file_size) {
        errno = EOVERFLOW;
        return 0;
    }
    size = min(size, file->dentry->file_size - offset);
    if (size == 0) {
        return 0;
    }
    off_t cluster_off;
    ssize_t bytes_read = 0, bytes_remaining = size, bytes_to_read_cluster = 0;
    u32 cluster =
        fat_table_seek_cluster(file->table, file->start_cluster, offset);
    if (errno != 0) {
        return 0;
    }

    while (bytes_remaining > 0) { // There are still bytes to read
        DEBUG("Next cluster to read %u", cluster);
        if (cluster == FAT_CLUSTER_END_OF_CHAIN) {
            break;
        }
        bytes_to_read_cluster = fat_table_get_cluster_remaining_bytes(
            file->table, bytes_remaining, offset);
        cluster_off = fat_table_cluster_offset(file->table, cluster) +
                      fat_table_mask_offset(offset, file->table);
        bytes_read = full_pread(file->table->fd, buf, bytes_to_read_cluster,
                                cluster_off);
        if (bytes_read != bytes_to_read_cluster) {
            break;
        }
        buf += bytes_read; // Move pointer
        offset += bytes_read;
        bytes_remaining -= bytes_read;
        cluster = fat_table_get_next_cluster(file->table, cluster);
    }
    fill_dentry_time_now(file->dentry, false, false);
    write_dir_entry(parent, file->dentry, file->pos_in_parent);
    return size - bytes_remaining;
}

void fat_file_truncate(fat_file file, off_t offset, fat_file parent) {
    u32 new_num_clusters = 0, current_num_clusters = 0;
    u32 last_cluster = 0, next_cluster = 0;

    current_num_clusters = max(1, fat_table_get_clusters_for_size(
                                      file->table, file->dentry->file_size));
    new_num_clusters =
        max(1, fat_table_get_clusters_for_size(file->table, offset));

    // Calculate how many clusters to remove
    if (offset > file->dentry->file_size ||
        new_num_clusters >= current_num_clusters) {
        return; // Nothing to truncate
    }
    // TODO [optional]
    // If the file size is smaller than length, bytes between the old and
    // new lengths are read as zeros.
    last_cluster =
        fat_table_seek_cluster(file->table, file->start_cluster, offset);
    if (errno != 0) {
        return;
    }

    next_cluster = fat_table_get_next_cluster(file->table, last_cluster);
    // Mark current cluster as the last one
    fat_table_set_next_cluster(file->table, last_cluster,
                               FAT_CLUSTER_END_OF_CHAIN);
    last_cluster = next_cluster;
    if (errno != 0) {
        return;
    }
    // Mark following clusters as not used
    while (!fat_table_is_EOC(file->table, last_cluster)) {
        // If there was an error, we continue with the function.
        next_cluster = fat_table_get_next_cluster(file->table, last_cluster);
        fat_table_set_next_cluster(
            file->table, last_cluster,
            FAT_CLUSTER_FREE); // last_cluster->next = "NULL"
        last_cluster = next_cluster;
    }

    // Update entrance in directory
    file->dentry->file_size = offset; // Overwrite with new size
    fill_dentry_time_now(file->dentry, false, true);
    write_dir_entry(parent, file->dentry, file->pos_in_parent);
}

// DOUBLE PRIZE: LIL ARCHAVAL "JAMAICAN GREEN" GOLD x2
void fat_file_remove(fat_file file, fat_file parent) {
    u32 curr_cluster = 0, next_cluster = 0;

    if (file->dentry->file_size == 0)
        goto miami;

    curr_cluster = fat_table_get_next_cluster(file->table, file->start_cluster);

    // Iterate over all clusters
    while (!fat_table_is_EOC(file->table,
                             curr_cluster)) { // while curr != FAT_CLUSTER_EOC
        next_cluster = fat_table_get_next_cluster(file->table, curr_cluster);
        fat_table_set_next_cluster(file->table, curr_cluster,
                                   FAT_CLUSTER_FREE); // curr_cluster = "NULL"
        curr_cluster = next_cluster;
    }

    fat_table_set_next_cluster(file->table, file->start_cluster,
                               FAT_CLUSTER_FREE);

    file->dentry->file_size = 0;
miami:
    file->dentry->base_name[0] = 0xe5;

    write_dir_entry(parent, file->dentry, file->pos_in_parent);
}

ssize_t fat_file_pwrite(fat_file file, const void *buf, size_t size,
                        off_t offset, fat_file parent) {
    u32 cluster = 0;
    ssize_t bytes_written_cluster = 0, bytes_remaining = size;
    ssize_t bytes_to_write_cluster = 0;
    off_t original_offset = offset, cluster_off = 0;

    if (offset > file->dentry->file_size) {
        errno = EOVERFLOW;
        return 0;
    }
    // Move cluster to first cluster to write
    cluster = fat_table_seek_cluster(file->table, file->start_cluster, offset);
    if (errno != 0) {
        return 0;
    }

    while (bytes_remaining > 0 && !fat_table_is_EOC(file->table, cluster)) {
        DEBUG("Next cluster to write %u", cluster);
        bytes_to_write_cluster = fat_table_get_cluster_remaining_bytes(
            file->table, bytes_remaining, offset);
        cluster_off = fat_table_cluster_offset(file->table, cluster) +
                      fat_table_mask_offset(offset, file->table);
        bytes_written_cluster = full_pwrite(
            file->table->fd, buf, bytes_to_write_cluster, cluster_off);
        bytes_remaining -= bytes_written_cluster;
        if (bytes_written_cluster != bytes_to_write_cluster) {
            break;
        }
        buf += bytes_written_cluster; // Move pointer
        offset += bytes_written_cluster;

        if (bytes_remaining > 0) {
            cluster = fat_table_get_next_cluster(file->table, cluster);
            if (errno != 0) {
                break;
            }
        }
    }

    // Update new file size
    if (original_offset + size - bytes_remaining > file->dentry->file_size) {
        file->dentry->file_size = offset + size - bytes_remaining;
    }
    // TODO if this operation fails, then the FAT table and the file's parent
    // entry are left on an incosistent state. FIXME
    // Update modified time
    fill_dentry_time_now(file->dentry, false, true);
    write_dir_entry(parent, file->dentry, file->pos_in_parent);

    return size - bytes_remaining;
}
