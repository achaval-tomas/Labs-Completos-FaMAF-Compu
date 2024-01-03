#include "big_brother.h"
#include <stdio.h>
#include <string.h>

int is_log_file_dentry(unsigned char *base_name, unsigned char *extension) {
    return strncmp(LOG_FILE_BASENAME, (char *)base_name, 3) == 0 &&
           strncmp(LOG_FILE_EXTENSION, (char *)extension, 3) == 0;
}

int is_log_filepath(char *filepath) {
    return strncmp(LOG_FILE, filepath, 8) == 0;
}
