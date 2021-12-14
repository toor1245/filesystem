#ifndef FS_FILE_H
#define FS_FILE_H

#include <stdbool.h>
#include <stdint.h>

#include "descriptor.h"
#include "linked_list.h"
#include "array_list.h"

typedef struct {
  fs_descriptor_t *fd;
  char *name;
  uint32_t content;
  bool is_link;
  bool is_opened;
  array_list_t *open_ids;
} file_t;

file_t *file_new(char *name, bool link);

void file_free(file_t *file);

file_t *file_find(linked_list_t *linked_list, int fd);

file_t *linked_list_file_find_by_name(linked_list_t *linked_list, char *name);

void linked_list_file_unlink(linked_list_t *linked_list, char *name);

void linked_list_remove(linked_list_t *linked_list, bool (*f)(int));

#endif // FS_FILE_H
