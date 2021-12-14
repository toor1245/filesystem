#include "internal/bit_utils.h"
#include "internal/bitmap.h"
#include "internal/filesystem_macros.h"
#include "internal/filesystem.h"
#include "memory.h"

#include "stdio.h"

filesystem_t *filesystem;

#define FS_SUCCESS 0
#define FS_FAILURE 1

#define FS_STORAGE_SIZE_IN_BYTES 1000

static void fs_new(int num_fd) {
  // format disk
  if (filesystem) {
    free(filesystem->bitmap->map);
    free(filesystem->bitmap);
    filesystem->bitmap = NULL;
    tree_delete_all(&filesystem->bst->ptr);
    filesystem->bst->ptr = NULL;
    free(filesystem->bst);
    filesystem->bst = NULL;
    linked_list_free(filesystem->files);
    filesystem->files = NULL;
    free(filesystem);
    filesystem = NULL;
  }
  filesystem = malloc(sizeof(filesystem_t));
  filesystem->max_num_fd = num_fd;
  filesystem->format = true;
  filesystem->mount = false;
}

static bool fs_enable_exec_command() {
  return filesystem && filesystem->format && filesystem->mount;
}

static void fs_create_directory_link(char *link) {
  if (fs_enable_exec_command() && filesystem->bst->ptr) {
    file_t *file = (file_t *) filesystem->files->head;
    linked_list_push(file->fd->links, link);
  }
}

int fs_create(char *name) {
  if (!filesystem || filesystem->num_files == filesystem->max_num_fd) {
    printf("Cannot create file");
  }
  if (!name || !fs_enable_exec_command()) {
    printf("Cannot create file, not created filesystem or unmounted\n");
    return FS_FAILURE;
  }

  file_t *found_file = linked_list_file_find_by_name(filesystem->files, name);
  if (found_file) {
    printf("File already exists\n");
    return FS_FAILURE;
  }

  file_t *file = file_new(name, false);
  file->fd = fs_descriptor_new(FS_FILE, 0);
  fs_create_directory_link(name);
  linked_list_push(filesystem->files, (void *) file);
  filesystem->num_files++;
  return FS_SUCCESS;
}

int fs_mkfs(int num_fd) {
  if (num_fd > FS_MAX_NUM_DESCRIPTORS) {
    printf("Size more than max size of descriptors: %d > %d\n", num_fd, FS_MAX_NUM_DESCRIPTORS);
    return FS_FAILURE;
  }
  fs_new(num_fd);
  printf("Created filesystem\n");
  return FS_SUCCESS;
}

int fs_ls() {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  node_t *current = filesystem->files->head;
  while (current) {
    file_t *file = (file_t *) current->value;
    printf("file: %s\n", file->name);
    current = current->next;
  }
  return FS_SUCCESS;
}

int fs_mount() {
  if (!filesystem || !filesystem->format) {
    printf("Filesystem not formatted\n");
    return FS_FAILURE;
  }
  filesystem->bitmap = bitmap_create((FS_STORAGE_SIZE_IN_BYTES / FS_BYTES_PER_BITMAP_BYTE) + 1);
  filesystem->bst = tree_new();
  filesystem->files = linked_list_new();
  filesystem->storage = malloc(sizeof(unsigned char) * FS_STORAGE_SIZE_IN_BYTES);

  file_t *file = file_new("root\\", false);
  file->fd = fs_descriptor_new(FS_DIRECTORY, 0);
  filesystem->mount = true;
  printf("Mounted\n");
  return FS_SUCCESS;
}

int fs_unmount() {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  free(filesystem->bitmap->map);
  free(filesystem->bitmap);
  filesystem->bitmap = NULL;
  linked_list_free(filesystem->files);
  filesystem->files = NULL;
  tree_delete_all(&filesystem->bst->ptr);
  filesystem->bst->ptr = NULL;
  free(filesystem->bst);
  filesystem->bst = NULL;
  filesystem->mount = false;
  printf("Unmounted\n");
  return FS_SUCCESS;
}

int fs_fstat(int id) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  node_t *current = filesystem->files->head;
  while (current) {
    file_t *file = (file_t *) current->value;
    if (file->fd->id == id) {
      fs_descriptor_show(file->fd);
      return FS_SUCCESS;
    }
    current = current->next;
  }
  return FS_FAILURE;
}

int fs_link(char *name1, char *name2) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = (file_t *) linked_list_file_find_by_name(filesystem->files, name2);
  if (file && file->is_link) {
    return FS_FAILURE;
  }
  linked_list_push(file->fd->links, name1);
  file->is_link = true;
  file_t *file_link = file_new(name1, true);
  file_link->fd = fs_descriptor_new(FS_FILE, file->fd->file_size);
  file_link->content = file->content;
  linked_list_push(filesystem->files, (void *) file_link);
  printf("file %s linked to %s\n", name1, name2);
  return FS_SUCCESS;
}

int fs_unlink(char *name) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = linked_list_file_find_by_name(filesystem->files, name);
  if (!file || !file->is_link) {
    return FS_FAILURE;
  }
  linked_list_file_unlink(filesystem->files, file->name);
  file_free(file);
  return FS_SUCCESS;
}

#define FS_OPEN_FILE_ID_SHIFT 0
#define FS_OPEN_FD_SHIFT      16

#define FS_CREATE_OPEN_FILE_FD(id, fd)  \
  (((id) << FS_OPEN_FILE_ID_SHIFT) |    \
   ((fd) << FS_OPEN_FD_SHIFT))

int fs_open(char *name) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = linked_list_file_find_by_name(filesystem->files, name);
  if (!file) {
    return FS_FAILURE;
  }

  if (file->is_link) {
    return FS_FAILURE;
  }
  file->is_opened = true;
  printf("created open id: %llu\n", FS_CREATE_OPEN_FILE_FD(file->fd->id, file->open_ids->size));
  array_list_push(file->open_ids, FS_CREATE_OPEN_FILE_FD(file->fd->id, file->open_ids->size)); // NOLINT
  return FS_SUCCESS;
}

inline static uint32_t fs_extract_open_file_id(int fd) {
  return extract_bit_range(fd, 15, 0);
}

inline static uint32_t fs_extract_open_fd(int fd) {
  return extract_bit_range(fd, 31, 16);
}

static file_t *fs_find_file(int fd) {
  node_t *current = filesystem->files->head;
  while (current) {
    file_t *file = (file_t *) current->value;
    if (file->fd->id == fs_extract_open_file_id(fd)) {
      return file;
    }
    current = current->next;
  }
  return NULL;
}

int fs_close(int fd) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  node_t *current = filesystem->files->head;
  while (current) {
    file_t *file = (file_t *) current->value;
    if (file->is_opened && file->fd->id == fs_extract_open_file_id(fd)) {
      array_list_remove_at(file->open_ids, fs_extract_open_fd(fd));
      if (file->open_ids->size == 0) {
        file->is_opened = false;
      }
      printf("fd %d closed\n", fd);
      return FS_SUCCESS;
    }
    current = current->next;
  }
  return FS_FAILURE;
}

static bool fs_file_is_opened(file_t *file, int fd) {
  if (!file || !file->is_opened) {
    return false;
  }
  for (int i = 0; i < file->open_ids->size; ++i) {
    if (fs_extract_open_fd(fd) == fs_extract_open_fd(file->open_ids->array[i])) {
      return true;
    }
  }
  return false;
}

int fs_read(int fd, uint32_t offset, uint32_t size) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = fs_find_file(fd);
  if (!fs_file_is_opened(file, fd)) {
    printf("File is not opened\n");
    return FS_FAILURE;
  }

  if (!file->fd->file_size) {
    return FS_FAILURE;
  }
  tree_node_t *tree_node = tree_find_node(filesystem->bst->ptr, file->content);
  if (!tree_node) {
    return FS_FAILURE;
  }
  uint32_t length = tree_node->num_reserved_bits * FS_BYTES_PER_BITMAP_BIT;
  if (size + offset >= length) {
    return FS_FAILURE;
  }
  unsigned char *content = &filesystem->storage[tree_node->index * FS_BYTES_PER_BITMAP_BIT];
  printf("%.*s\n", size, content);
  return FS_SUCCESS;
}

int fs_write(int fd, char *buffer, uint32_t offset, uint32_t size) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = fs_find_file(fd);
  if (!fs_file_is_opened(file, fd)) {
    printf("File is not opened\n");
    return FS_FAILURE;
  }

  uint32_t run_bits = (size / FS_BYTES_PER_BITMAP_BIT) + 1;
  int32_t index = bitmap_get_bit_run(filesystem->bitmap, run_bits);
  if (index == -1) return FS_FAILURE;
  bitmap_set_bits(filesystem->bitmap, 0, run_bits, index);

  tree_node_t *tree_node = malloc(sizeof(tree_node_t));
  tree_node->index = index;
  tree_node->num_reserved_bits = run_bits;

  uint32_t storage_index = index * FS_BYTES_PER_BITMAP_BIT;
  uint32_t copy_size = run_bits * FS_BYTES_PER_BITMAP_BIT;
  if (storage_index + offset + copy_size >= FS_STORAGE_SIZE_IN_BYTES) {
    return FS_FAILURE;
  }
  memmove(&filesystem->storage[storage_index], &buffer[0], copy_size);
  uint32_t storage_location = (uint32_t) ((intptr_t) (&filesystem->storage[storage_index]));
  file->content = storage_location;
  file->fd->file_size = size;
  tree_node->value = storage_location;
  tree_node->name = file->name;
  tree_insert_node(&filesystem->bst->ptr, tree_node);
  printf("Write file %s\n", file->name);
  return FS_SUCCESS;
}

int fs_truncate(char *name, uint32_t size) {
  if (!fs_enable_exec_command()) {
    printf("Not mounted or formatted\n");
    return FS_FAILURE;
  }
  file_t *file = linked_list_file_find_by_name(filesystem->files, name);
  if (!file) {
    return FS_FAILURE;
  }
  tree_node_t *tree_node = tree_find_node(filesystem->bst->ptr, file->content);
  if (!tree_node) {
    return FS_FAILURE;
  }
  if (size == 0) {
    bitmap_set_bits(filesystem->bitmap, 1, tree_node->num_reserved_bits, tree_node->index);
    tree_delete_node(&filesystem->bst->ptr, file->content);
  }
  if (tree_node->num_reserved_bits * FS_BYTES_PER_BITMAP_BIT == size) {
    return FS_SUCCESS;
  }
  uint32_t temp_size = tree_node->num_reserved_bits * FS_BYTES_PER_BITMAP_BIT;
  bitmap_set_bits(filesystem->bitmap, 1, tree_node->num_reserved_bits, tree_node->index);
  uint32_t run_bits = (size / FS_BYTES_PER_BITMAP_BIT) + 1;
  int32_t index = bitmap_get_bit_run(filesystem->bitmap, run_bits);
  if (index == -1) return FS_FAILURE;
  bitmap_set_bits(filesystem->bitmap, 0, run_bits, index);

  tree_node->index = index;
  tree_node->num_reserved_bits = run_bits;

  uint32_t storage_index = index * FS_BYTES_PER_BITMAP_BIT;
  if (temp_size < size) {
    uint32_t temp_index = temp_size - 1;
    for (uint32_t i = storage_index + temp_index; i < size - 1; ++i) {
      filesystem->storage[i] = '\0';
    }
  }
  file->fd->file_size = size;
  return FS_SUCCESS;
}
