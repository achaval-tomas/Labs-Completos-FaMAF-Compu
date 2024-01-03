/*
 * fat_file.h
 *
 * This data sctructure is an abstraction of a file in a FAT file system.
 * It contains a copy of the directory entry that describes the file, and
 * other useful information, like a counter of times the file was opened or a
 * reference to the fat_table where it's stored.
 *
 * It also abstracts the functions to read and write the directory entries
 * into fat_table.
 */

#ifndef _FAT_FILE_H
#define _FAT_FILE_H

#include "fat_types.h"
#include <gmodule.h>
#include <sys/types.h>
#include <utime.h>

struct stat;

/* Flags that go in the @attribs field of FAT directory entries. */
#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_HIDDEN 0x00000002
#define FILE_ATTRIBUTE_SYSTEM 0x00000004
#define FILE_ATTRIBUTE_VOLUME 0x00000008
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#define FILE_ATTRIBUTE_DEVICE 0x00000040
#define FILE_ATTRIBUTE_RESERVED 0x00000080

/* Filename; with extension, if present; null-terminated */
#define MAX_FILENAME (8 + 1 + 3 + 1)
#define MAX_PATH_LEN 4096 /* Copied from libfat */

/********************* DATA STRUCTURES *********************/

/* Format of a FAT directory entry on disk (32 bytes) */
struct fat_dir_entry_s {
    u8 base_name[8];
    u8 extension[3];
    u8 attribs;
    u8 reserved;
    u8 create_time_fine_res;
    le16 create_time;      // BEWARE: little endian number
    le16 create_date;      // BEWARE: little endian number
    le16 last_access_date; // BEWARE: little endian number
    union {
        le16 file_access_bitmap; // Old formats
        le16 fat32_start_cluster_high;
    };
    le16 last_modified_time; // BEWARE: little endian number
    le16 last_modified_date; // BEWARE: little endian number
    le16 start_cluster_low;  // only last two bytes
    le32 file_size;
} __attribute__((packed));

/* Wrapper around a FAT directory entry that contains members used to put it in
 * data structures, and to use it as a file handle */
struct fat_file_s {
    // The data from the actual FAT directory entry
    fat_dir_entry dentry;
    // Full name of the file (with extension)
    char name[MAX_FILENAME];
    // Full path to file.
    char *filepath;
    // Full start cluster
    u32 start_cluster;

    union {
        // Valid only for directories
        struct {
            // Number of consecutive dir entries in disk (including ignored
            // ones). It also marks the position of the first free space for a
            // dir_entry.
            u32 nentries;
        } dir;
        // Valid only for non-directory files
        struct {
            // Number of clusters that this file is supposed to be,
            // based on the file_size given in the directory entry.
            u32 num_clusters;
        } file;
    };
    // Position in the parent directory entry table
    u32 pos_in_parent;
    // Pointer to the FAT table containing this file
    fat_table table;
    // Current number of open file descriptors to this file
    u32 num_times_opened : 31;
    // True iff the subdirectories of this file have been read into memory.
    // Always 0 for non-directories.
    u32 children_read : 1;
};

/* Returns an new directory entry with default values.
 * Caller is still owner of @filepath reference.
 */
fat_dir_entry fat_file_init_direntry(bool is_dir, char *filepath,
                                     u32 start_cluster);

/* Inits a file without direntry. Can be used to create root file.
 * TAD is owner of @filepath and will apply free at destroy.
 */
fat_file fat_file_init_empty(bool is_dir, char *filepath);

/* Allocate memory and do common initializations on a `fat_file'.
 * Set's the file in the next free entry of @table, and updates
 * If fat_table_get_next_free_cluster(vol) fails or is inconsistent, sets errno
 * to ENOSPC. If set_next_cluster fails, sets errno to EIO.
 */
fat_file fat_file_init(fat_table table, bool is_dir, char *filepath);

/* Frees filepath and dentry fields of @file, and finally the fat_file_s itself.
 */
void fat_file_destroy(fat_file file);

/* Returns strcmp between the filepath of @file1 and @file2. */
int fat_file_cmp(fat_file file1, fat_file file2);

/* Returns strcmp between the filepath of @file1 and filepath. */
int fat_file_cmp_path(fat_file file1, char *filepath);

/********************* FILE METADATA *********************/

/* Returns true if @file is a directory. */
bool fat_file_is_directory(const fat_file file);

/* Increment the number of times that a FAT file or directory has been opened */
void fat_file_inc_num_times_opened(fat_file file);

/* Decrement the number of times that a FAT file or directory has been opened */
void fat_file_dec_num_times_opened(fat_file file);

/* Transfer the attributes of a FAT file into standard UNIX format struct stat.
 */
void fat_file_to_stbuf(fat_file file, struct stat *stbuf);

/* Logging functions. Prints dentry information.*/
void fat_file_print_dentry(fat_dir_entry dentry);

/* Fills @buf with the time information in @file. */
void fat_utime(fat_file file, fat_file parent, const struct utimbuf *buf);

/********************* DIRECTORY FUNCTIONS *********************/

/* Adds the directory entry of @child to directory @parent, writing the
 * FAT table. The file is added to the first free direntry space of @parent.
 * If there is no more space in the parent's data cluster, sets errno to ENOSPC.
 * If there is an error in the write operation, sets errno to EIO.
 */
void fat_file_dentry_add_child(fat_file parent, fat_file child);

/* Creates fat_file instances from @dir's directory entries in the FAT table.
 * Returns a GList with references to newly created fat_file.
 * It is not recursive (does not read files in subdirectories).
 * If there is an error in the read operation, sets errno to EIO and returns
 * NULL. Expects @dir to be a directory, not a file.*/
GList *fat_file_read_children(fat_file dir);

/********************* DATA OPERATIONS *********************/

/* Read @size bytes from the FAT file @file at offset @offset, storing the
 * result into the buffer @buf. Returns a negative error number on failure,
 * otherwise the number of bytes read. The number of bytes read will be less
 * than size only in case EOF was encountered before finishing read operation.
 * If offset is greater than file size, does not read any data and
 * sets errno to EOVERFLOW.
 * If there is an error in the read or write operations, sets errno to EIO.
 */
ssize_t fat_file_pread(fat_file file, void *buf, size_t size, off_t offset,
                       fat_file parent);

/* Truncates @file to @offset bytes. Frees unused clusters and sets new file
 * size. If offset is greater than file size, no operation is performed.
 * If there is an error in the read or write operations, sets errno to EIO
 */
void fat_file_truncate(fat_file file, off_t offset, fat_file parent);

/*
 *  Sets all clusters occupied by @file to FREE. (0x00000000)
 */
void fat_file_remove(fat_file file, fat_file parent);

/* Write @size bytes from the FAT file @file at offset @offset, reading from
 * result into the buffer @buf. Returns a negative error number on failure,
 * otherwise the number of bytes written (short count only on EOF).
 * If offset is greater than file size, does not read any data and
 * sets errno to EOVERFLOW (althoug this is not the expected behaviour of
 * write functions) TODO fill with zeros the gap between file_size and offset.
 * If there is an error in the read or write operations, sets errno to EIO.
 */
ssize_t fat_file_pwrite(fat_file file, const void *buf, size_t size,
                        off_t offset, fat_file parent);

#endif /* _FAT_FILE_H */
