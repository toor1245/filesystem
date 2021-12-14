#ifndef FILESYSTEM_ARRAY_LIST_H
#define FILESYSTEM_ARRAY_LIST_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint32_t *array;
  uint32_t size;
  uint32_t length;
} array_list_t;

array_list_t *array_list_new();

void array_list_push(array_list_t *array_list, uint32_t item);

int array_list_index_of(array_list_t *array_list, uint32_t item);

void array_list_remove_at(array_list_t *array_list, uint32_t index);

#endif // FILESYSTEM_ARRAY_LIST_H
