#include "array_list.h"
#include "assert.h"
#include "memory.h"
#include "filesystem_macros.h"

#ifndef FS_COMPILER_CLANG
#error "target compiler is not clang"
#endif

#define ARRAY_LIST_DEFAULT_CAPACITY 10
#define ARRAY_LIST_MAX_LENGTH 10000

array_list_t *array_list_new() {
  array_list_t *list = (array_list_t *) malloc(sizeof(array_list_t));
  list->size = 0;
  list->array = malloc(sizeof(uint32_t) * ARRAY_LIST_DEFAULT_CAPACITY);
  for (int i = 0; i < ARRAY_LIST_DEFAULT_CAPACITY; ++i) list->array[i] = 0;
  list->length = ARRAY_LIST_DEFAULT_CAPACITY;
  return list;
}

void array_list_set_capacity(array_list_t *array_list, uint32_t value) {
  assert(value > array_list->size);
  if (value != array_list->length) {
    if (value > 0) {
      uint32_t *new_array = malloc(sizeof(uint32_t) * value);
      if (array_list->size > 0) {
        memmove(&new_array[0], &array_list->array[0], sizeof(uint32_t) * array_list->size);
      }
      array_list->array = new_array;
      array_list->length = value;
    } else {
      array_list->array = NULL;
      array_list->size = 0;
      array_list->length = 0;
    }
  }
}

void array_list_ensure_capacity(array_list_t *array_list, uint32_t min) {
  if (array_list->length < min) {
    uint32_t new_capacity = array_list->length == 0 ? ARRAY_LIST_DEFAULT_CAPACITY : array_list->length * 2;
    if (new_capacity > ARRAY_LIST_MAX_LENGTH) new_capacity = ARRAY_LIST_MAX_LENGTH;
    if (new_capacity < min) new_capacity = min;
    array_list_set_capacity(array_list, new_capacity);
  }
}

void array_list_push(array_list_t *array_list, uint32_t item) {
  if (array_list->size == array_list->length) {
    array_list_ensure_capacity(array_list, array_list->size + 1);
  }
  array_list->array[array_list->size++] = item;
}

void array_list_remove_at(array_list_t *array_list, uint32_t index) {
  if (!array_list) return;
  assert(index < array_list->size);
  array_list->size--;
  if (index < array_list->size) {
    uint32_t src_index = index + 1;
    memmove(&array_list->array[index], &array_list->array[src_index], sizeof(uint32_t) * (array_list->size - index));
  }
  array_list->array[array_list->size] = 0;
}
