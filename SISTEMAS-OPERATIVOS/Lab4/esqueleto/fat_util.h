#ifndef _FAT_UTIL_H
#define _FAT_UTIL_H

#include "fat_types.h"
#include <sys/types.h>

#define min(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })

#define max(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b;      \
    })

#ifndef NDEBUG
#include <errno.h>
#include <stdio.h>
#define DEBUG(format, ...)                                             \
    ({                                                                 \
        int errno_save = errno;                                        \
        fprintf(stderr, "%s:%u %s(): ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, format, ##__VA_ARGS__);                        \
        putc('\n', stderr);                                            \
        errno = errno_save;                                            \
    })
#else
#define DEBUG(format, ...)
#endif

/* Reads @count bytes from @fd into @buf, starting at offset @offset. Returns
 * the number of bytes effectively read.
 * Like pread(), but keep trying until everything has been read or we know for
 * sure that there was an error (or end-of-file).
 * If there is an error in the read operation, sets errno to EIO.
 */
size_t full_pread(int fd, void *buf, size_t count, off_t offset);

/* Writes @count bytes from @buf into @fd, starting at offset @offset. Returns
 * the number of bytes effectively read.
 * Like pwrite(), but keep trying until everything has been read or we know for
 * sure that there was an error (or end-of-file)
 * If there is an error in the write operations, sets errno to EIO.
 */
size_t full_pwrite(int fd, const void *buf, size_t count, off_t offset);

/* Print an error message. */
void fat_error(const char *format, ...);

/* Strip trailing spaces from a string */
void remove_trailing_spaces(char *s);

/* Saves in @entry_date and @entry_time the time entries for an *in-disk*
 * file structure, using the time @t. If @entry_time is NULL, is not filled.
 */
int fill_time(le16 *entry_date, le16 *entry_time, const time_t t);

/* Given a 16-bit FAT date and 16-bit FAT time in the eccentric FAT format,
 * return a standard UNIX time (seconds since January 1, 1970).
 */
time_t time_to_unix_time(u16 le_date, u16 le_time);

/* Cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:    the type of the container struct this is embedded in.
 * @member:    the name of the member within the struct.
 */
#define container_of(ptr, type, member)                        \
    ({                                                         \
        const __typeof__(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member));     \
    })

/* Bit-scan-reverse:  Return the position of the highest bit set in @n, indexed
 * from bit 0 as the low bit.  @n cannot be 0. */
static inline unsigned long bsr(unsigned long n) {
    __asm__("bsr %1,%0" : "=r"(n) : "rm"(n));
    return n;
}

/* Returns %true iff @n is a power of 2.  Zero is not a power of 2. */
static inline bool is_power_of_2(size_t n) {
    return (n & (n - 1)) == 0 && n != 0;
}

#endif /* _FAT_UTIL_H */
