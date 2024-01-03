#ifndef MOCK_FAT_FILE_H
#define MOCK_FAT_FILE_H

#include <stdbool.h>

typedef char *fat_file;
typedef void *fat_volume;

fat_file fat_file_init(fat_volume vol, bool is_dir, char *filepath);

void fat_file_destroy(fat_file file);

int fat_file_cmp(fat_file file1, fat_file file2);

int fat_file_cmp_path(fat_file file1, char *filepath);

void fat_file_inc_num_times_opened(fat_file file);

void fat_file_dec_num_times_opened(fat_file file);

#endif /* MOCK_FAT_FILE_H */