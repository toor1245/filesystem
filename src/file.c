#include "file.h"
#include "linked_list.h"
#include "string.h"

void file_free(file_t *file) {
  if (!file) return;
  if (file->fd) {
    linked_list_free(file->fd->links);
    free(file->fd);
  }
  free(file);
}

file_t *file_new(char *name, bool link) {
  file_t *file = (file_t *) malloc(sizeof(file_t));
  file->name = name;
  file->is_link = link;
  file->open_ids = array_list_new();
  file->fd = NULL;
  file->parent_dir = NULL;
  file->content = 0;
  file->is_opened = false;
  return file;
}

file_t *linked_list_file_find_by_name(linked_list_t *linked_list, char *name) {
  node_t *current = linked_list->head;
  while (current) {
    file_t *file = (file_t *) current->value;
    if (strcmp(file->name, name) == 0) {
      return file;
    }
    current = current->next;
  }
  return NULL;
}

void linked_list_file_unlink(linked_list_t *linked_list, char *name) {
  node_t *current = linked_list->head;
  node_t *previous = NULL;

  while (current) {
    file_t *file = (file_t *) current->value;
    if (strcmp(file->name, name) == 0) {
      if (previous) {
        previous->next = current->next;
        if (!current->next) {
          linked_list->tail = previous;
        }
      } else {
        linked_list->head = linked_list->head->next;
        if (!linked_list->head) {
          linked_list->tail = NULL;
        }
      }
      linked_list->count--;
    }
    previous = current;
    current = current->next;
  }
}