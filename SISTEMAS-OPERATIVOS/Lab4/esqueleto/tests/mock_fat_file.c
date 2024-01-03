#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mock_fat_file.h"

fat_file fat_file_init(fat_volume vol, bool is_dir, char *filepath) {
    return filepath;
}

void fat_file_destroy(fat_file file) { free(file); }

int fat_file_cmp(fat_file file1, fat_file file2) {
    return strcmp(file1, file2);
}

int fat_file_cmp_path(fat_file file1, char *filepath) {
    return strcmp(file1, filepath);
}

void fat_file_inc_num_times_opened(fat_file file) { file[0] = 'X'; }

void fat_file_dec_num_times_opened(fat_file file) { file[0] = 'X'; }