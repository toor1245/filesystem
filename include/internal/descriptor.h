#ifndef FILESYSTEM_DESCRIPTOR_H
#define FILESYSTEM_DESCRIPTOR_H

#include "stdint.h"
#include "linked_list.h"

typedef enum {
  FS_FILE,
  FS_DIRECTORY
} fs_type_t;

typedef struct {
  size_t id;
  fs_type_t type;
  linked_list_t *links;
  int32_t file_size; // file size in bytes
} fs_descriptor_t;

fs_descriptor_t *fs_descriptor_new(fs_type_t type, int32_t file_size);

void fs_descriptor_show(fs_descriptor_t *descriptor);

#endif // FILESYSTEM_DESCRIPTOR_H
