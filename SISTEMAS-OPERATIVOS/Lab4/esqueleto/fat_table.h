/*
 * fat_table.h
 *
 * The fat_table TAD is an abstraction the can be used to navigate, reand and
 * write the FAT table and the chains of data clusters.
 */
#ifndef _FAT_TABLE_H
#define _FAT_TABLE_H

#include "fat_types.h"
#include "fat_util.h"
#include <sys/types.h>

// Both values of EOC are valid in FAT32 systems
#define FAT_CLUSTER_END_OF_CHAIN 0x0FFFFFF8
#define FAT_CLUSTER_END_OF_CHAIN2 0x0FFFFFFF
#define FAT_CLUSTER_FREE 0x00000000

/* Abstraction of the fat table that handles cluster information and
 * read/write operations
 */
struct fat_table_s {
    // FAT mapped into memory
    void *fat_map;
    // The offset to write the fat table
    off_t fat_offset;
    // Number of data clusters
    u32 num_data_clusters;
    // Byte offset of the "second" cluster (first in the actual layout)
    off_t data_start_offset;
    // Open file descriptor to the volume file or device
    int fd;
    u16 cluster_order;
};

bool fat_table_is_valid_cluster_number(const fat_table table, u32 cluster);

/* Get the next cluster in the chain of clusters for @cur_cluster.
 * Returns FAT_CLUSTER_END_OF_CHAIN if this was the last cluster in the chain;
 * otherwise returns the number of the next cluster.
 */
u32 fat_table_get_next_cluster(fat_table table, u32 cur_cluster);

/* Calculates the number of bytes per cluster based on the cluster order
 * (this is the power of two needed to do so)
 */
size_t fat_table_bytes_per_cluster(fat_table table);

/* Masks the offset so when subtracting it from the max bytes on the
 * cluster it doesn't overflow
 */
size_t fat_table_mask_offset(off_t offset, fat_table table);

/* Calculate how many of @bytes_remaining can be or read/write in a cluster,
 * starting from @offset position.
 */
size_t fat_table_get_cluster_remaining_bytes(fat_table table,
                                             size_t bytes_remaining,
                                             off_t offset);

/* Calculates the number of clusters necessary to fit @size bytes. */
u32 fat_table_get_clusters_for_size(fat_table table, size_t file_size);

/* Returns the number of the first unused cluster in the data sector. */
u32 fat_table_get_next_free_cluster(fat_table table);

/* Returns the offset in bytes to the address where @cluster starts. */
off_t fat_table_cluster_offset(const fat_table table, u32 cluster);

/* Returns true if @cluster is not marked as free. */
bool fat_table_is_cluster_used(fat_table table, u32 cluster);

/* In the fat table, writes the entry of @cur_cluster marking that the next
 * cluster in chain is @next_cluster.
 * If there is an error on the write operation, sets errno to EIO
 */
void fat_table_set_next_cluster(fat_table table, u32 cur_cluster,
                                u32 next_cluster);

/* Gets the number of the cluster that's @offset bytes in the chain
 * starting on @start_cluster.
 * If EOC is encountered first, returns 0 and sets errno to EOVERFLOW.
 */
u32 fat_table_seek_cluster(fat_table table, u32 start_cluster, off_t offset);

/* Returns true if @cluster is the end of the cluster chain */
bool fat_table_is_EOC(fat_table table, u32 cluster);

/* Prints the index and cluster value (as ints) stored in @table, from
 * @start_cluster to @end_cluster.
 * Useful for debugging.
 */
void fat_table_print(fat_table table, u32 start_cluster, u32 end_cluster);

#endif /* _FAT_TABLE_H */
