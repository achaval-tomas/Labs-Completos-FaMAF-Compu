#ifndef _FAT_VOLUME_H
#define _FAT_VOLUME_H

#include "fat_file.h"
#include "fat_fs_tree.h"
#include "fat_table.h"
#include "fat_types.h"
#include <sys/types.h>

#define FAT_MOUNT_FLAG_READONLY 0x1
#define FAT_MOUNT_FLAG_READWRITE 0x2

struct fat_volume_s {
    fat_table table;
    // Flags passed to fat_volume_mount()
    int mount_flags;
    // Tree of directories and files. In memory structure
    fat_tree file_tree;
    // Maximum number of `struct fat_file_s's to allocate (soft limit only)
    size_t max_allocated_files;
    // Standard boot sector info
    char oem_name[8 + 1];
    // Data from DOS 2.0 BIOS Parameter Block
    u16 bytes_per_sector;
    u16 sector_order;
    u8 sectors_per_cluster;
    u8 sectors_per_cluster_order;
    u16 reserved_sectors;
    u8 num_tables;
    u16 max_root_entries;
    u8 media_descriptor;
    u32 total_sectors;
    u32 sectors_per_fat; // 16-bit in DOS 2.0 BPB, 32-bit in FAT32 EBPB
    // Data from DOS 3.0 BIOS Parameter Block
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    // Data from FAT32 Extended BIOS parameter block
    u16 drive_description;
    u16 version;
    u32 root_dir_start_cluster;
    u16 fs_info_sector;
    u16 alt_boot_sector;
    // Data from non-FAT32 Extended BIOS parameter block
    u8 physical_drive_num;
    u8 extended_boot_sig;
    u32 volume_id;
    char volume_label[11 + 1];
    char fs_type[8 + 1];
};

/* Open a FAT volume and prepare it for mounting by returning a
 * fat_volme containing information read from the filesystem superblock, and
 * a reference to the fat_fs_tree directory tree. The root directory is
 * already inserted on the tree.
 */
fat_volume fat_volume_mount(const char *volume, int mount_flags);

/* Unmount FAT volume @vol */
int fat_volume_unmount(fat_volume vol);

#endif /* _FAT_VOLUME_H */
