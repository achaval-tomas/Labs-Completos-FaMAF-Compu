#ifndef _BIG_BROTHER_H
#define _BIG_BROTHER_H

#define LOG_FILE "/fs.log"
#define LOG_FILE_BASENAME "fs"
#define LOG_FILE_EXTENSION "log"

int is_log_file_dentry(unsigned char *base_name, unsigned char *extension);

int is_log_filepath(char *filepath);

#endif