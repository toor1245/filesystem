#ifndef FILESYSTEM_LINKED_LIST_H
#define FILESYSTEM_LINKED_LIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct node {
  void *value;
  struct node *next;
} node_t;

typedef struct {
  node_t *head;
  node_t *tail;
  uint32_t count;
} linked_list_t;

static void linked_list_push(linked_list_t *linked_list, void *value) {
  node_t *node = malloc(sizeof(node_t));
  node->next = NULL;
  node->value = value;
  if (!linked_list->head) {
    linked_list->head = node;
  } else {
    linked_list->tail->next = node;
  }
  linked_list->tail = node;
  linked_list->count++;
}

static void linked_list_free(linked_list_t *linked_list) {
  if (!linked_list) return;
  node_t *temp;
  while (linked_list->head) {
    temp = linked_list->head;
    linked_list->head = linked_list->head->next;
    free(temp);
  }
  linked_list->head = NULL;
  linked_list->tail = NULL;
  linked_list->count = 0;
}

static linked_list_t *linked_list_new() {
  linked_list_t *linked_list = malloc(sizeof(linked_list_t));
  linked_list->head = NULL;
  linked_list->tail = NULL;
  linked_list->count = 0;
  return linked_list;
}

static void linked_list_foreach(linked_list_t *linked_list, void (*func)(void *)) {
  node_t *current = linked_list->head;
  while (current) {
    func(current);
    current = current->next;
  }
}

static bool linked_list_foreach_arg(linked_list_t *linked_list, bool (*func)(void *, void *), void *arg) {
  node_t *current = linked_list->head;
  while (current) {
    if (func(current, arg)) {
      return true;
    }
    current = current->next;
  }
  return false;
}

static bool linked_list_foreach_is_success(linked_list_t *linked_list, bool (*func)(void *)) {
  node_t *current = linked_list->head;
  while (current) {
    if (!func(current)) {
      return false;
    }
    current = current->next;
  }
  return true;
}

static bool linked_list_remove_if(linked_list_t *linked_list, bool (*func)(void *)) {
  node_t *current = linked_list->head;
  node_t *previous = NULL;
  while (current) {
    if (func(current)) {
      if (previous) {
        previous->next = current->next;
        if (!current->next)
          linked_list->tail = previous;
      } else {
        linked_list->head = linked_list->head->next;
        if (!linked_list->head) {
          linked_list->tail = NULL;
        }
      }
      linked_list->count--;
      return true;
    }
    previous = current;
    current = current->next;
  }
  return false;
}

#define CREATE_LINKED_LIST_FOREACH_FIRST_TYPE(TYPE)                                                                     \
static bool linked_list_foreach_first_arg_##TYPE(linked_list_t *linked_list, bool (*func)(void *, TYPE), TYPE value) {  \
  node_t *current = linked_list->head;                                                                                  \
  while (current) {                                                                                                     \
    if (func(current, value)) {                                                                                         \
      return true;                                                                                                      \
    }                                                                                                                   \
    current = current->next;                                                                                            \
  }                                                                                                                     \
  return false;                                                                                                         \
}

CREATE_LINKED_LIST_FOREACH_FIRST_TYPE(int)
CREATE_LINKED_LIST_FOREACH_FIRST_TYPE(char)
CREATE_LINKED_LIST_FOREACH_FIRST_TYPE(bool)

#define CREATE_LINKED_LIST_FOREACH_FIRST_NODE_TYPE(TYPE)                                                                       \
static node_t* linked_list_foreach_first_node_arg_##TYPE(linked_list_t *linked_list, bool (*func)(void *, TYPE), TYPE value) { \
  node_t *current = linked_list->head;                                                                                         \
  while (current) {                                                                                                            \
    if (func(current, value)) {                                                                                                \
      return current;                                                                                                          \
    }                                                                                                                          \
    current = current->next;                                                                                                   \
  }                                                                                                                            \
  return NULL;                                                                                                                 \
}

CREATE_LINKED_LIST_FOREACH_FIRST_NODE_TYPE(int)

#endif // FILESYSTEM_LINKED_LIST_H
