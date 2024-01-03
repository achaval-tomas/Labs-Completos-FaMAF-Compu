/*
 * fat_table.c
 *
 * The fat_table TAD is an abstraction the can be used to navigate, reand and
 * write the FAT table and the chains of data clusters.
 */

#include "fat_table.h"
#include <unistd.h>

inline bool fat_table_is_valid_cluster_number(const fat_table table,
                                              u32 cluster) {
    return cluster >= 2 && cluster < table->num_data_clusters + 2;
}

u32 fat_table_get_next_cluster(fat_table table, u32 cur_cluster) {
    u32 next_cluster;
    next_cluster = le32_to_cpu(((const le32 *)table->fat_map)[cur_cluster]);

    /* We currently don't check for the actual special cluster values, but
     * instead treat all out of range values as end-of-chain. This may be
     * okay for read-only mounts. */
    if (!fat_table_is_valid_cluster_number(table, next_cluster)) {
        next_cluster = FAT_CLUSTER_END_OF_CHAIN;
    }
    return next_cluster;
}

inline size_t fat_table_bytes_per_cluster(fat_table table) {
    return 1 << table->cluster_order;
}

size_t fat_table_mask_offset(off_t offset, fat_table table) {
    size_t bytes_per_cluster = fat_table_bytes_per_cluster(table);
    return offset & (bytes_per_cluster - 1);
}

size_t fat_table_get_cluster_remaining_bytes(fat_table table,
                                             size_t bytes_remaining,
                                             off_t offset) {
    size_t bytes_per_cluster = fat_table_bytes_per_cluster(table);
    size_t masked_offset = fat_table_mask_offset(offset, table);
    return min(bytes_per_cluster - masked_offset, bytes_remaining);
}

u32 fat_table_get_clusters_for_size(fat_table table, size_t file_size) {
    size_t bytes_per_cluster = fat_table_bytes_per_cluster(table);
    return ((off_t)file_size + (bytes_per_cluster - 1)) >> table->cluster_order;
}

u32 fat_table_get_next_free_cluster(fat_table table) {
    u32 fat_table_get_next_free_cluster =
        2; /* First two clusters are reserved */
    while (
        le32_to_cpu(
            ((const le32 *)table->fat_map)[fat_table_get_next_free_cluster]) !=
        FAT_CLUSTER_FREE) {
        fat_table_get_next_free_cluster++;
    }
    if (!fat_table_is_valid_cluster_number(table,
                                           fat_table_get_next_free_cluster)) {
        fat_error("There was a problem fetching for a free cluster");
        fat_table_get_next_free_cluster = FAT_CLUSTER_END_OF_CHAIN;
    }
    DEBUG("next free cluster = %u", fat_table_get_next_free_cluster);
    return fat_table_get_next_free_cluster;
}

inline off_t fat_table_cluster_offset(const fat_table table, u32 cluster) {
    return table->data_start_offset +
           ((off_t)(cluster - 2) << table->cluster_order);
}

inline bool fat_table_is_cluster_used(fat_table table, u32 cluster) {
    return le32_to_cpu(((const le32 *)table->fat_map)[cluster]) != 0;
}

void fat_table_set_next_cluster(fat_table table, u32 cur_cluster,
                                u32 next_cluster) {
    le32 next_cluster_le32 = cpu_to_le32(next_cluster);
    /* Write the disk fat table */
    off_t entry_offset = (off_t)(cur_cluster * 4) + table->fat_offset;
    ssize_t written_bytes =
        pwrite(table->fd, &next_cluster_le32, sizeof(le32), entry_offset);
    if (written_bytes <= 0) {
        DEBUG("Error writing next cluster disk entry");
        errno = EIO;
        return;
    }
    /* Alter the in-memory table */
    ((le32 *)table->fat_map)[cur_cluster] = next_cluster_le32;
}

u32 fat_table_seek_cluster(fat_table table, u32 start_cluster, off_t offset) {
    u32 positions_to_move = offset >> table->cluster_order;
    // Move start_cluster to first cluster to read
    for (u32 i = 0; i < positions_to_move; ++i) {
        if (start_cluster == FAT_CLUSTER_END_OF_CHAIN) {
            DEBUG("Offset is bigger than file's last cluster.");
            errno = EOVERFLOW;
            return 0;
        }
        start_cluster = fat_table_get_next_cluster(table, start_cluster);
    }
    return start_cluster;
}

bool fat_table_is_EOC(fat_table table, u32 cluster) {
    return cluster == FAT_CLUSTER_END_OF_CHAIN ||
           cluster == FAT_CLUSTER_END_OF_CHAIN2;
}

void fat_table_print(fat_table table, u32 start_cluster, u32 end_cluster) {
    u32 cur_cluster = start_cluster, counter = 50;
    while (counter >= 0 && cur_cluster != end_cluster) {
        if (fat_table_is_EOC(
                table,
                le32_to_cpu(((const le32 *)table->fat_map)[cur_cluster]))) {
            printf("|[%u]EOC", cur_cluster);
        } else {
            printf("|[%u]%u", cur_cluster,
                   ((le32 *)table->fat_map)[cur_cluster]);
        }
        cur_cluster++;
        counter--;
    }
    printf("|\n");
}