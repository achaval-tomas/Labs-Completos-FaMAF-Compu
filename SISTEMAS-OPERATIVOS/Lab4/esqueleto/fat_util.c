#include "fat_util.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

size_t full_pread(int fd, void *buf, size_t count, off_t offset) {
    ssize_t bytes_read;
    size_t bytes_remaining;

    for (bytes_remaining = count; bytes_remaining != 0;
         bytes_remaining -= bytes_read, buf += bytes_read,
        offset += bytes_read) {
        bytes_read = pread(fd, buf, bytes_remaining, offset);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                errno = EIO;
            } else if (errno == EINTR) {
                continue;
            }
            break;
        }
    }
    return count - bytes_remaining;
}

size_t full_pwrite(int fd, const void *buf, size_t count, off_t offset) {
    ssize_t bytes_read;
    size_t bytes_remaining;

    for (bytes_remaining = count; bytes_remaining != 0;
         bytes_remaining -= bytes_read, buf += bytes_read,
        offset += bytes_read) {
        bytes_read = pwrite(fd, buf, bytes_remaining, offset);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                errno = EIO;
            } else if (errno == EINTR) {
                continue;
            }
            break;
        }
    }
    return count - bytes_remaining;
}

void fat_error(const char *format, ...) {
    va_list va;

    fputs("fat-fuse: ", stderr);
    va_start(va, format);
    vfprintf(stderr, format, va);
    putc('\n', stderr);
    va_end(va);
}

void remove_trailing_spaces(char *s) {
    char *p = strchr(s, '\0');
    while (--p >= s && *p == ' ') {
        *p = '\0';
    }
}

/* Adapted from libfat. */
int fill_time(le16 *entry_date, le16 *entry_time, const time_t t) {
    struct tm time_st;
    le16 date = 0;
    le16 tim = 0;
    le16 bmask3 = 0x07FF;
    le16 bmask2 = 0x01FF;
    le16 bmask1 = 0x001F;

    gmtime_r(&t, &time_st);

    date = (le16)time_st.tm_mday;
    date &= bmask1; // to set 0 first 11 bits
    date |= ((le16)time_st.tm_mon + 1) << 5;
    date &= bmask2; // to set 0 first 6 bits
    date |= (((le16)((time_st.tm_year + 1900) - 1980)) << 9);
    *entry_date = cpu_to_le16(date);

    if (entry_time != NULL) {
        tim = (le16)(time_st.tm_sec / 2);
        tim &= bmask1;
        tim |= (((le16)(time_st.tm_min)) << 5);
        tim &= bmask3;
        tim |= (((le16)(time_st.tm_hour)) << 11);

        *entry_time = cpu_to_le16(tim);
    }
    return 0;
}

time_t time_to_unix_time(u16 le_date, u16 le_time) {
    u16 date = le16_to_cpu(le_date), time = le16_to_cpu(le_time);
    // FAT dates are years since 1980.  mktime() expects years since 1900.
    u16 year = (date >> 9) + (1980 - 1900);
    // FAT months are numbered 1-12; mktime() expects months numbered 0-11.
    u16 month = ((date >> 5) & 0xf) - 1;
    // Both FAT days and mktime() mdays are numbered 1-31.
    u16 day = date & 0x1f;
    // Hours (0-23)
    u16 hours = time >> 11;
    // Minutes (0-59)
    u16 minutes = (time >> 5) & 0x3f;
    // Seconds after minute (0-59).  FAT counts 2-second intervals, so
    // multiply by 2.
    u16 seconds = (time & 0x1f) * 2;

    struct tm tm = {
        .tm_sec = seconds,
        .tm_min = minutes,
        .tm_hour = hours,
        .tm_mday = day,
        .tm_mon = month,
        .tm_year = year,
        .tm_wday = 0,
        .tm_yday = 0,
        .tm_isdst = -1,
    };
    return mktime(&tm);
}
