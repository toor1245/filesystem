#include "internal/bit_utils.h"
#include "internal/bitmap.h"
#include "internal/filesystem_macros.h"
#include "internal/filesystem.h"
#include "internal/file_path.h"

#include <memory.h>
#include <string.h>
#include <stdio.h>

filesystem_t *filesystem;
file_t *cwd;

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

#define FS_ENABLE_EXECUTION()             \
  if (!fs_enable_exec_command()) {        \
    printf("Not mounted or formatted\n"); \
    return FS_FAILURE;                    \
  }

static bool file_move_to_dir(void *data, void *token) {
  path_token_t *path_token = (path_token_t *) token;
  file_t *file = (file_t *) ((node_t *) data)->value;
  if (file->fd->type != FS_DIRECTORY) {
    return false;
  }
  if (strcmp(path_token->value, file->name) == 0) {
    cwd = file;
    return true;
  }
  return false;
}

static bool path_token_cd(void *data) {
  path_token_t *path_token = (path_token_t *) ((node_t *) data)->value;
  if (PATH_FILE == path_token->type) {
    bool is_changed_dir = linked_list_foreach_arg(cwd->fd->links, file_move_to_dir, path_token);
    if (!is_changed_dir && !path_token->is_last) {
      return false;
    }
    return true;
  }
  if (PATH_PARENT == path_token->type) {
    if (!cwd->parent_dir) {
      return false;
    }
    cwd = cwd->parent_dir;
    return true;
  }
  return true;
}

static bool try_change_dir(file_t *original_cwd, path_parse_t* path_parse) {
  if (path_parse == NULL) {
    return false;
  }
  bool is_success = linked_list_foreach_is_success(path_parse->token_types, path_token_cd);
  if (!is_success) {
    cwd = original_cwd;
    return false;
  }
  return true;
}

int fs_create(char *path) {
  if (!filesystem || filesystem->num_files == filesystem->max_num_fd) {
    printf("Cannot create file");
  }
  if (!path || !fs_enable_exec_command()) {
    printf("Cannot create file, not created filesystem or unmounted\n");
    return FS_FAILURE;
  }

  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *found_file = linked_list_file_find_by_name(cwd->fd->links, path_parse->name);
  if (found_file) {
    cwd = original_cwd;
    printf("File already exists\n");
    return FS_FAILURE;
  }

  file_t *file = file_new(path_parse->name, false);
  file->fd = fs_descriptor_new(FS_FILE, 0);
  linked_list_push(filesystem->files, (void *) file);
  linked_list_push(cwd->fd->links, file);
  filesystem->num_files++;
  cwd = original_cwd;
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

static void ls_print(void *data) {
  file_t *file = (file_t *) ((node_t *) data)->value;
  printf("file: %s\n", file->name);
}

int fs_ls() {
  FS_ENABLE_EXECUTION()
  linked_list_foreach(cwd->fd->links, ls_print);
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

  file_t *file = file_new("root", false);
  file->fd = fs_descriptor_new(FS_DIRECTORY, 0);
  filesystem->mount = true;
  cwd = file;
  printf("Mounted\n");
  return FS_SUCCESS;
}

int fs_unmount() {
  FS_ENABLE_EXECUTION()
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

static bool fstat(void *data, int id) {
  file_t *file = (file_t *) ((node_t *) data)->value;
  if (file->fd->id == id) {
    fs_descriptor_show(file->fd);
    return true;
  }
  return false;
}

int fs_fstat(int id) {
  FS_ENABLE_EXECUTION()
  linked_list_foreach_first_arg_int(filesystem->files, fstat, id);
  return FS_FAILURE;
}

int fs_link(char *path1, char *path2) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path2);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *file = (file_t *) linked_list_file_find_by_name(cwd->fd->links, path_parse->name);
  if (file && file->is_link) {
    return FS_FAILURE;
  }
  file->is_link = true;
  file_t *file_link = file_new(path1, true);
  file_link->fd = fs_descriptor_new(FS_FILE, file->fd->file_size);
  file_link->content = file->content;
  linked_list_push(file->fd->links, file_link);
  linked_list_push(filesystem->files, (void *) file_link);
  printf("file %s linked to %s\n", path1, path2);
  return FS_SUCCESS;
}

int fs_unlink(char *name) {
  FS_ENABLE_EXECUTION()
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

int fs_open(char *path) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *file = linked_list_file_find_by_name(cwd->fd->links, path_parse->name);
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

static bool find_file(void *data, int fd) {
  file_t *file = (file_t *) ((node_t *) data)->value;
  if (file->fd->id == fs_extract_open_file_id(fd)) {
    return true;
  }
  return false;
}

static bool close_fd(void *data, int fd) {
  file_t *file = (file_t *) ((node_t *) data)->value;
  if (file->is_opened && file->fd->id == fs_extract_open_file_id(fd)) {
    array_list_remove_at(file->open_ids, fs_extract_open_fd(fd));
    if (file->open_ids->size == 0) {
      file->is_opened = false;
    }
    printf("fd %d closed\n", fd);
    return true;
  }
  return false;
}

int fs_close(int fd) {
  FS_ENABLE_EXECUTION()
  return linked_list_foreach_first_arg_int(cwd->fd->links, close_fd, fd);
}

static bool fs_file_is_opened(file_t *file, int fd) {
  if (!file->is_opened) {
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
  FS_ENABLE_EXECUTION()
  node_t *node = linked_list_foreach_first_node_arg_int(filesystem->files, find_file, fd);
  if (!node) {
    return FS_FAILURE;
  }
  file_t *file = (file_t *) node->value;
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
  FS_ENABLE_EXECUTION()
  node_t *node = linked_list_foreach_first_node_arg_int(filesystem->files, find_file, fd);
  if (!node) {
    return FS_FAILURE;
  }
  file_t *file = (file_t *) node->value;
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

int fs_truncate(char *path, uint32_t size) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *file = linked_list_file_find_by_name(cwd->fd->links, path_parse->name);
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

int fs_cd(char *path) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  return FS_SUCCESS;
}

int fs_mkdir(char *path) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *sub_directory = file_new(path_parse->name, false);
  sub_directory->fd = fs_descriptor_new(FS_DIRECTORY, 0);
  sub_directory->parent_dir = cwd;
  if (!cwd->parent_dir) {
    linked_list_push(cwd->fd->links, (void *) sub_directory);
  } else {
    linked_list_push(cwd->parent_dir->fd->links, (void *) sub_directory);
  }
  cwd = original_cwd;
  return FS_SUCCESS;
}

static bool remove_empty_dir(void *data) {
  file_t *file = (file_t *) ((node_t *) data)->value;
  return file->fd->type == FS_DIRECTORY && !file->fd->links->count;
}

int fs_rmdir(char *path) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  linked_list_remove_if(cwd->parent_dir->fd->links, remove_empty_dir);
  cwd = original_cwd;
  return FS_SUCCESS;
}

int fs_symlink(char *str, char *path) {
  FS_ENABLE_EXECUTION()
  path_parse_t *path_parse = file_path_parse(path);
  file_t *original_cwd = cwd;
  if (!try_change_dir(original_cwd, path_parse)) {
    return FS_FAILURE;
  }
  file_t *file = file_new(str, false);
  file->fd = fs_descriptor_new(FS_SYMLINK, 0);
  linked_list_push(file->fd->links, cwd);
  linked_list_push(filesystem->files, (void *) file);
  cwd = original_cwd;
  return FS_SUCCESS;
}
