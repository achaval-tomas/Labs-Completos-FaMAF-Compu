/*
 * fat_volume.c
 *
 * Read filesystem superblock and file allocation table.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "fat_util.h"
#include "fat_volume.h"

/* Extended BIOS Parameter Block (non-FAT-32 version) */
struct nonfat32_ebpb {
    u8 physical_drive_num;
    u8 reserved;
    u8 extended_boot_sig;
    le32 volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed));

/* Extended BIOS Parameter Block (FAT-32 version) */
struct fat32_ebpb {
    le32 sectors_per_fat;
    le16 drive_description;
    le16 version;
    le32 root_dir_start_cluster;
    le16 fs_info_sector;
    le16 alt_boot_sector;
    u8 reserved[12];
} __attribute__((packed));

/* On-disk format of the FAT boot sector, starting from the very first byte of
 * the device. */
struct fat_boot_sector_disk {
    // Jump instruction for the bootloader (ignored)
    u8 jump_insn[3];

    // Standard boot sector info
    char oem_name[8];

    // DOS 2.0 BIOS Parameter Block (13 bytes)
    le16 bytes_per_sector;
    u8 sectors_per_cluster;
    le16 reserved_sectors;
    u8 num_tables;
    le16 max_root_entries;
    le16 total_sectors;
    u8 media_descriptor;
    le16 sectors_per_fat;

    // DOS 3.31 BIOS Parameter Block (12 bytes)
    le16 sectors_per_track;
    le16 num_heads;
    le32 hidden_sectors;
    le32 total_sectors_32;

    // Extended BIOS Parameter Block (only FAT32
    struct __attribute__((packed)) {
        struct fat32_ebpb fat32_ebpb;
        struct nonfat32_ebpb nonfat32_ebpb;
    } fat32;
} __attribute__((packed));

/* Read DOS 2.0-compatible "BIOS Parameter Block" (13 bytes) */
static int read_dos_2_0_bpb(fat_volume vol,
                            const struct fat_boot_sector_disk *boot_sec) {
    vol->bytes_per_sector = le16_to_cpu(boot_sec->bytes_per_sector);
    vol->sector_order = bsr(vol->bytes_per_sector);

    // Make sure the sector size in bytes is a power of 2 between 32 and
    // 4096, inclusive
    if (!is_power_of_2(vol->bytes_per_sector) || vol->sector_order < 5 ||
        vol->sector_order > 12) {
        fat_error("bytes_per_sector (%hu) is invalid", vol->bytes_per_sector);
        errno = EINVAL;
        return -1;
    }

    DEBUG("sector_order = %hu (%hu bytes per sector)", vol->sector_order,
          vol->bytes_per_sector);

    // Make sure the sectors per cluster is a power of 2 between 1 and 128,
    // inclusive
    vol->sectors_per_cluster = boot_sec->sectors_per_cluster;
    vol->sectors_per_cluster_order = bsr(vol->sectors_per_cluster);

    if (!is_power_of_2(vol->sectors_per_cluster) ||
        vol->sectors_per_cluster_order > 7) {
        fat_error("sectors_per_cluster (%hhu) is invalid",
                  vol->sectors_per_cluster);
        errno = EINVAL;
        return -1;
    }
    vol->table->cluster_order =
        vol->sector_order + vol->sectors_per_cluster_order;

    DEBUG("cluster_order = %hu (%hhu sectors = %u bytes per cluster)",
          vol->table->cluster_order, vol->sectors_per_cluster,
          1 << vol->table->cluster_order);

    // Number of reserved sectors
    vol->reserved_sectors = le16_to_cpu(boot_sec->reserved_sectors);

    // Number of FATs must be 1 or 2
    vol->num_tables = boot_sec->num_tables;
    if (vol->num_tables != 1 && vol->num_tables != 2) {
        fat_error("num_tables (%hhu) is invalid", vol->num_tables);
        errno = EINVAL;
        return -1;
    }

    vol->max_root_entries = le16_to_cpu(boot_sec->max_root_entries);
    if (vol->max_root_entries != 0) {
        // Max root directory entries is 0: this indicates a FAT32
        // volume (root directory is stored in ordinary data clusters)
        fat_error("Root directory entries not 0 implies invalid FAT format:"
                  " Not FAT32");
    }

    // Total logical sectors (2 bytes); if 0 use 4 byte value later
    vol->total_sectors = le16_to_cpu(boot_sec->total_sectors);
    vol->media_descriptor = boot_sec->media_descriptor;
    vol->sectors_per_fat = le16_to_cpu(boot_sec->sectors_per_fat);
    return 0;
}

/* Read DOS 3.31-compatible "BIOS Parameter Block" (12 bytes) */
static int read_dos_3_31_bpb(fat_volume vol,
                             const struct fat_boot_sector_disk *boot_sec) {
    vol->sectors_per_track = le16_to_cpu(boot_sec->sectors_per_track);
    vol->num_heads = le16_to_cpu(boot_sec->num_heads);
    vol->hidden_sectors = le32_to_cpu(boot_sec->hidden_sectors);

    return 0;
}

/* Read generic "Extended BIOS Parameter Block" */
static int read_nonfat32_ebpb(fat_volume vol,
                              const struct nonfat32_ebpb *ebpb) {
    vol->physical_drive_num = ebpb->physical_drive_num;
    vol->extended_boot_sig = ebpb->extended_boot_sig;
    vol->volume_id = le32_to_cpu(ebpb->volume_id);
    memcpy(vol->volume_label, ebpb->volume_label, sizeof(ebpb->volume_label));
    memcpy(vol->fs_type, ebpb->fs_type, sizeof(ebpb->fs_type));
    remove_trailing_spaces(vol->volume_label);
    remove_trailing_spaces(vol->fs_type);

    return 0;
}

/* Read FAT32-specific "Extended BIOS Parameter Block" */
static int read_fat32_ebpb(fat_volume vol, const struct fat32_ebpb *ebpb) {
    if (le32_to_cpu(ebpb->sectors_per_fat) != 0) {
        vol->sectors_per_fat = le32_to_cpu(ebpb->sectors_per_fat);
        if ((((size_t)vol->sectors_per_fat << vol->sector_order) >>
             vol->sector_order) != (size_t)vol->sectors_per_fat) {
            // Size of FAT in bytes does not fit in 'size_t'.
            fat_error("Unexpectedly high sectors per FAT (%u)",
                      vol->sectors_per_fat);
            errno = EINVAL;
            return -1;
        }
    }
    vol->drive_description = le16_to_cpu(ebpb->drive_description);
    vol->version = le16_to_cpu(ebpb->version);
    if (vol->version != 0) {
        fat_error("Unknown filesystem version %hu; refusing to mount",
                  vol->version);
        errno = EINVAL;
        return -1;
    }
    vol->root_dir_start_cluster = le32_to_cpu(ebpb->root_dir_start_cluster);
    if (vol->root_dir_start_cluster == 0) {
        fat_error("Invalid starting cluster for root directory");
        errno = EINVAL;
        return -1;
    }
    vol->fs_info_sector = le16_to_cpu(ebpb->fs_info_sector);
    vol->alt_boot_sector = le16_to_cpu(ebpb->alt_boot_sector);
    DEBUG("root_dir_start_cluster = %u", vol->root_dir_start_cluster);

    if (vol->fs_info_sector == 0xffff) {
        vol->fs_info_sector = 0;
    }
    if (vol->fs_info_sector != 0 && vol->sector_order < 9) {
        fat_error("FS Information Sector is present, but "
                  "sector size is less than 512 bytes");
        errno = EINVAL;
        return -1;
    }
    return 0;
}

/* Read the data from the FAT filesystem's boot sector, located at the beginning
 * of the file given by the file descriptor @fd, into the `struct fat_volume_s'
 * @vol.  Return 0 on success; returns -1 and sets errno on failure.
 */
static int read_boot_sector(fat_volume vol, int fd) {
    u8 buf[128]; // Variable to read from file descriptor, is at most 79 bytes
    const struct fat_boot_sector_disk *boot_sec;
    int ret;
    u32 num_data_sectors;

    DEBUG("Reading FAT boot sector");

    if (full_pread(fd, buf, 128, 0) != 128)
        return -1;

    boot_sec = (struct fat_boot_sector_disk *)buf;
    memcpy(vol->oem_name, boot_sec->oem_name, sizeof(boot_sec->oem_name));
    remove_trailing_spaces(vol->oem_name);

    ret = read_dos_2_0_bpb(vol, boot_sec);
    if (ret)
        return ret;

    ret = read_dos_3_31_bpb(vol, boot_sec);
    if (ret)
        return ret;

    ret = read_fat32_ebpb(vol, &boot_sec->fat32.fat32_ebpb);
    if (ret)
        return ret;
    ret = read_nonfat32_ebpb(vol, &boot_sec->fat32.nonfat32_ebpb);
    if (ret)
        return ret;

    num_data_sectors = vol->total_sectors - vol->reserved_sectors -
                       ((vol->max_root_entries << 5) >> vol->sector_order);
    vol->table->num_data_clusters =
        num_data_sectors >> vol->sectors_per_cluster_order;
    DEBUG("num_data_clusters = %u", vol->table->num_data_clusters);
    if (vol->table->num_data_clusters < 65535) {
        fat_error("Too few data clusters imply invalid FAT format: Not FAT32");
    }
    return 0;
}

/* Map the first File Allocation Table into memory. */
static int map_fat(fat_volume vol, int fd, int mount_flags) {
    long page_size;
    void *ptr;
    size_t fat_size_bytes;
    size_t fat_aligned_size_bytes;
    off_t fat_offset;
    off_t fat_aligned_offset;
    int prot;

    page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0 || !is_power_of_2(page_size) || page_size > (1U << 31)) {
        // Weird page size (shouldn't happen)
        fat_error("Invalid system page size (%ld)", page_size);
        errno = EINVAL;
        return -1;
    }

    fat_offset = (off_t)vol->reserved_sectors << vol->sector_order;
    fat_aligned_offset = fat_offset & ~(page_size - 1);

    fat_size_bytes = (size_t)vol->sectors_per_fat << vol->sector_order;
    fat_aligned_size_bytes = fat_size_bytes + (fat_offset - fat_aligned_offset);

    if (fat_aligned_size_bytes < fat_size_bytes) {
        // Unsigned integer overflow
        fat_error("FAT size too large");
        errno = EINVAL;
        return -1;
    }

    DEBUG("Mapping FAT into memory (%zu bytes at offset %zu)", fat_size_bytes,
          fat_offset);

    prot = PROT_READ;
    if (mount_flags & FAT_MOUNT_FLAG_READWRITE) {
        prot |= PROT_WRITE;
    }

    ptr = mmap(NULL, fat_aligned_size_bytes, prot, MAP_PRIVATE, fd,
               fat_aligned_offset);
    if (ptr == MAP_FAILED)
        return -1;
    vol->table->fat_map = ptr + (fat_offset - fat_aligned_offset);
    vol->table->fat_offset = fat_offset;
    return 0;
}

static fat_file init_root_dir(fat_volume vol) {
    fat_dir_entry root_entry =
        fat_file_init_direntry(true, strdup("/"), vol->root_dir_start_cluster);
    fat_file root_dir = fat_file_init_empty(true, strdup("/"));
    if (errno != 0) {
        free(root_entry);
        free(root_dir);
        free(vol);
        return NULL;
    }
    root_dir->dentry = root_entry;
    root_dir->start_cluster = vol->root_dir_start_cluster; // Only FAT32
    root_dir->table = vol->table;
    return root_dir;
}

static fat_volume init_file_tree(fat_volume vol) {
    fat_file root_dir = init_root_dir(vol);
    vol->file_tree = fat_tree_init();
    vol->file_tree = fat_tree_insert(vol->file_tree, NULL, root_dir);
    return vol;
}

fat_volume fat_volume_mount(const char *volume, int mount_flags) {
    fat_volume vol = NULL;
    int fd;
    int ret;
    int open_flags;

    DEBUG("Mounting FAT volume \"%s\"", volume);

    vol = calloc(1, sizeof(struct fat_volume_s));
    vol->table = calloc(1, sizeof(struct fat_table_s));
    if (!vol)
        return vol;

    if (mount_flags & FAT_MOUNT_FLAG_READWRITE) {
        open_flags = O_RDWR;
    } else {
        open_flags = O_RDONLY;
    }

    fd = open(volume, open_flags);
    if (fd < 0) {
        free(vol);
        vol = NULL;
        return vol;
    }

    // Read the FAT boot sector
    ret = read_boot_sector(vol, fd);
    if (ret) {
        close(fd);
        free(vol);
        vol = NULL;
        return vol;
    }

    // Map the first File Allocation Table into memory
    ret = map_fat(vol, fd, mount_flags);
    if (ret) {
        close(fd);
        free(vol);
        vol = NULL;
        return vol;
    }

    // Initialize other fields of the `struct fat_volume_s'
    vol->table->fd = fd; // File descriptor to use when reading data
    vol->mount_flags = mount_flags;
    // Arbitrary soft limit, to keep memory usage down.
    vol->max_allocated_files = 100;

    // Compute the offset of the first byte of the data area so it doesn't
    // need to be re-calculated over and over.
    vol->table->data_start_offset =
        (off_t)(vol->num_tables * vol->sectors_per_fat + vol->reserved_sectors +
                (vol->max_root_entries >> (vol->sector_order - 5)))
        << vol->sector_order;
    DEBUG("data_start_offset = %" PRIu64, vol->table->data_start_offset);

    // Initialize the root directory
    vol = init_file_tree(vol);
    return vol;
}

int fat_volume_unmount(fat_volume vol) {
    int ret;

    DEBUG("Unmounting FAT volume");

    ret = close(vol->table->fd);
    munmap(vol->table->fat_map,
           (size_t)vol->sectors_per_fat << vol->sector_order);
    fat_tree_destroy(vol->file_tree);
    free(vol);
    return ret;
}