#ifndef FILESYSTEM_FS_DRIVER_H
#define FILESYSTEM_FS_DRIVER_H

#include "stdbool.h"

#include "file.h"
#include "internal/linked_list.h"
#include "binary_tree.h"
#include "bitmap.h"

typedef struct {
  bitmap_t *bitmap;
  tree_t *bst;
  linked_list_t *files;
  uint32_t max_num_fd;
  uint32_t num_files;
  bool format;
  bool mount;
  unsigned char *storage;
} filesystem_t;

int fs_mkfs(int num_fd);

int fs_mount();

int fs_unmount();

int fs_fstat(int id);

int fs_ls();

int fs_create(char *path);

int fs_link(char *path1, char *path2);

int fs_unlink(char *name);

int fs_truncate(char *path, uint32_t size);

int fs_open(char *path);

int fs_close(int fd);

int fs_read(int fd, uint32_t offset, uint32_t size);

int fs_write(int fd, char *buffer, uint32_t offset, uint32_t size);

int fs_cd(char *path);

int fs_mkdir(char *path);

int fs_rmdir(char *path);

int fs_symlink(char* str, char* path);

#endif //FILESYSTEM_FS_DRIVER_H
