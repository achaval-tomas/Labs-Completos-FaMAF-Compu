#ifndef _FAT_TYPES_H
#define _FAT_TYPES_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* Little-endian numbers that cannot be used without endianness conversion */
typedef u16 le16;
typedef u32 le32;
typedef u64 le64;

typedef struct fat_dir_entry_s *fat_dir_entry;
typedef struct fat_file_s *fat_file;
typedef struct fat_table_s *fat_table;
typedef struct fat_volume_s *fat_volume;

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define cpu_to_le16(n) (n)
#define cpu_to_le32(n) (n)
#define cpu_to_le64(n) (n)

#define le16_to_cpu(n) (n)
#define le32_to_cpu(n) (n)
#define le64_to_cpu(n) (n)

#else /* __BYTE_ORDER == __LITTLE_ENDIAN */

#define cpu_to_le16(n) __builtin_bswap16(n)
#define cpu_to_le32(n) __builtin_bswap32(n)
#define cpu_to_le64(n) __builtin_bswap64(n)

#define le16_to_cpu(n) __builtin_bswap16(n)
#define le32_to_cpu(n) __builtin_bswap32(n)
#define le64_to_cpu(n) __builtin_bswap64(n)

#endif /* __BYTE_ORDER != __LITTLE_ENDIAN */

#endif /* _FAT_TYPES_H */
