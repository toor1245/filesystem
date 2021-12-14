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
    free(temp->value);
    free(temp);
  }
  linked_list->tail = NULL;
  linked_list->count = 0;
}

static linked_list_t *linked_list_new() {
  linked_list_t *linked_list = malloc(sizeof(linked_list_t *));
  linked_list->count = 0;
  linked_list->head = NULL;
  linked_list->tail = NULL;
  return linked_list;
}

#endif // FILESYSTEM_LINKED_LIST_H
