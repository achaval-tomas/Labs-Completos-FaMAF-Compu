#ifndef _FAT_FILENAME_UTIL_H
#define _FAT_FILENAME_UTIL_H

#include "fat_file.h"
#include "fat_types.h"
#include <string.h>
#include <sys/types.h>

#define FAT_FILENAME_DELETED_CHAR 0xe5

/* Compares the bytes of @s1 and @s2.
 * FAT names can be no more than 12 characters long, so we might as well just
 * inline strcmp() to make comparisons faster. */
int inline_strcmp(const char *s1, const char *s2);

/* Returns true iff @base_name of up to 8 bytes is valid. */
bool file_basename_valid(const u8 base_name[8]);

/* Returns true if @extension of up to 3 bytes is valid. */
bool file_extension_valid(const u8 extension[3]);

/* Return the actual length of a FAT base name or extension that may be up to
 * @max_len bytes long. Treats space and null character as ending the name.
 * If there is no ending character in name up to max_len position, the function
 * returns 0.*/
unsigned filename_len(const char *name, unsigned max_len);

/* Copies the filename into @dst_name_p. The destination pointer
 * must have enough memory reserved to hold the name (MAX_FILENAME).
 * If the filename has an extension, the result is (basename + . + extension).
 * otherwise it's just the basename. */
void build_filename(const u8 *src_name_p, const u8 *src_extension_p,
                    char *dst_name_p);

/* Converts the filename into a 8 character name and a up to 3 character
 * extension. Returns the base and the extension with u8 type, compatible with
 * fat_dir_entry.
 */
void filename_from_path(char *src_name_p, u8 *base, u8 *extension);

/* Returns a new allocated string with the full filepath for file
 * @file_name in directory @parent_filepath. The caller owns the reference.
 */
char *filepath_from_name(char *parent_filepath, char *file_name);

#endif /* _FAT_FILENAME_UTIL_H */
