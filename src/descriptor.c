#include <stdio.h>
#include "descriptor.h"

size_t global_fd_id; // used for generation unique fd_id

fs_descriptor_t *fs_descriptor_new(fs_type_t type, int32_t file_size) {
  fs_descriptor_t *fs_descriptor = malloc(sizeof(fs_descriptor_t));
  fs_descriptor->links = linked_list_new();
  fs_descriptor->type = type;
  fs_descriptor->id = global_fd_id++;
  fs_descriptor->file_size = file_size;
  return fs_descriptor;
}

void fs_descriptor_show(fs_descriptor_t *descriptor) {
  if (!descriptor) return;
  printf("id: %zu\n", descriptor->id);
  printf("file_size_in_bytes: %d\n", descriptor->file_size);
  if (descriptor->links && descriptor->links->count) {
    printf("links:\n");
    node_t *current = descriptor->links->head;
    while (current) {
      printf("link: %s", ((char *) (current->value)));
      current = current->next;
    }
    printf("\n");
  }
  switch (descriptor->type) {
    case FS_DIRECTORY:
      printf("type: DIRECTORY\n");
      break;
    case FS_FILE:
      printf("type: FILE\n");
      break;
    default:
      break;
  }
}
