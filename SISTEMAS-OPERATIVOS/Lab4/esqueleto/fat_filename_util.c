#include "fat_filename_util.h"
#include <stdio.h>

/* Flag indicating that the character is legal to use in a filename */
#define FAT_CHAR_LEGAL_IN_FILENAME 0x1
#define PATH_SEPARATOR "/"

/* Table of 1-byte characters and their interpretations in the FAT filesystem.
 */
static const unsigned char _fat_char_tab[256] = {
    ['a' ... 'z'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['A' ... 'Z'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['0' ... '9'] = FAT_CHAR_LEGAL_IN_FILENAME,
    [128 ... 255] = FAT_CHAR_LEGAL_IN_FILENAME,
    [' '] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['$'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['%'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['-'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['_'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['@'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['~'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['`'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['!'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['('] = FAT_CHAR_LEGAL_IN_FILENAME,
    [')'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['{'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['}'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['^'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['#'] = FAT_CHAR_LEGAL_IN_FILENAME,
    ['&'] = FAT_CHAR_LEGAL_IN_FILENAME,
};

inline int inline_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++, s2++;
    }
    return *s1 - *s2;
}

static bool char_legal_in_filename(char c) {
    return (_fat_char_tab[(unsigned char)c] & FAT_CHAR_LEGAL_IN_FILENAME) != 0;
}

bool file_basename_valid(const u8 base_name[8]) {
    unsigned i;
    if (base_name[0] == '\0' || base_name[0] == ' ' || base_name[0] == 0xe5) {
        // End of directory, or name starts with space, or free directory entry
        return false;
    }

    // Make sure all remaining characters are legal
    i = 0;
    do {
        if (!char_legal_in_filename(base_name[i]))
            return false;
        i++;
    } while (i != 8 && base_name[i] != '\0' && base_name[i] != ' ');
    return true;
}

bool file_extension_valid(const u8 extension[3]) {
    unsigned i;
    for (i = 0; i < 3 && extension[i] != '\0' && extension[i] != ' '; i++) {
        if (!char_legal_in_filename(extension[i])) {
            return false;
        }
    }
    return true;
}

unsigned filename_len(const char *name, unsigned max_len) {
    unsigned len = max_len;
    do {
        if (name[len - 1] != ' ' && name[len - 1] != '\0')
            break;
        len--;
    } while (len > 0);
    return len;
}

void build_filename(const u8 *src_name_p, const u8 *src_extension_p,
                    char *dst_name_p) {
    unsigned name_len;
    unsigned extension_len;
    int max_length = 8;
    // Get the base name of the file or directory
    name_len = filename_len((char *)src_name_p, max_length);
    if (name_len == 0) {
        *dst_name_p = '/';
        dst_name_p++;
        *dst_name_p = '\0';
        return;
    }
    do {
        *dst_name_p++ = *src_name_p++;
    } while (--name_len);

    // Append extension, if present, to the base name
    extension_len = filename_len((char *)src_extension_p, 3);
    if (extension_len) {
        *dst_name_p++ = '.';
        src_name_p = src_extension_p;
        do {
            *dst_name_p++ = *src_name_p++;
        } while (--extension_len);
    }
    *dst_name_p = '\0';
}

void filename_from_path(char *src_name_p, u8 *base, u8 *extension) {
    unsigned name_len = filename_len(src_name_p, 8);
    if (name_len == 0) { // Empty filename
        *base = '\0';
        *extension = '\0';
        return;
    }
    int i = 0, j = 0;
    char *dot_pos = strrchr(src_name_p, '.');
    do {
        base[i] = src_name_p[i];
        i++;
    } while (i < name_len && (src_name_p + i) != dot_pos);
    if (dot_pos != NULL && i < name_len) { // There is an extension to copy
        i++;                               // Skip dot position
        while (src_name_p[i] != ' ' && src_name_p[i] != '\0') {
            extension[j] = src_name_p[i];
            i++;
            j++;
        }
    }            // else: no dot => empty extension
    if (j < 3) { // extension was too short, fill with end of string
        extension[j + 1] = '\0';
    }
}

char *filepath_from_name(char *parent_filepath, char *file_name) {
    unsigned filepath_len = strlen(parent_filepath);
    unsigned name_len = strlen(file_name);
    char *filepath = calloc(filepath_len + name_len + 1, sizeof(char));
    strcpy(filepath, parent_filepath);
    if (parent_filepath[filepath_len - 1] != PATH_SEPARATOR[0]) {
        strcat(filepath, PATH_SEPARATOR);
    }
    strcat(filepath, file_name);
    return filepath;
}
