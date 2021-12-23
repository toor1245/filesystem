#ifndef FS_FILE_H
#define FS_FILE_H

#include <stdbool.h>
#include <stdint.h>

#include "descriptor.h"
#include "linked_list.h"
#include "array_list.h"

typedef struct file {
  fs_descriptor_t *fd;
  char *name;
  uint32_t content;
  bool is_link;
  bool is_opened;
  array_list_t *open_ids;
  struct file *parent_dir;
} file_t;

file_t *file_new(char *name, bool link);

void file_free(file_t *file);

file_t *linked_list_file_find_by_name(linked_list_t *linked_list, char *name);

void linked_list_file_unlink(linked_list_t *linked_list, char *name);

#endif // FS_FILE_H
