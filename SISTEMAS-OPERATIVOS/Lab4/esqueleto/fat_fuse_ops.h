#ifndef _FAT_FS_OPS_H
#define _FAT_FS_OPS_H

#include "fat_fs_tree.h"
#include <fuse/fuse.h>

int fat_fuse_fgetattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi);
int fat_fuse_getattr(const char *path, struct stat *stbuf);
int fat_fuse_open(const char *path, struct fuse_file_info *fi);
int fat_fuse_opendir(const char *path, struct fuse_file_info *fi);
int fat_fuse_mkdir(const char *path, mode_t mode);
int fat_fuse_mknod(const char *path, mode_t mode, dev_t dev);
int fat_fuse_read(const char *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi);
int fat_fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi);
int fat_fuse_release(const char *path, struct fuse_file_info *fi);
int fat_fuse_releasedir(const char *path, struct fuse_file_info *fi);
int fat_fuse_utime(const char *path, struct utimbuf *buf);
int fat_fuse_truncate(const char *path, off_t offset);
int fat_fuse_write(const char *path, const char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi);
int fat_fuse_unlink(const char *path);
int fat_fuse_rmdir(const char *path);

#endif /* _FAT_FS_OPS_H */