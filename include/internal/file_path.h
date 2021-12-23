#ifndef FILESYSTEM_FILE_PATH_H
#define FILESYSTEM_FILE_PATH_H

#include <stdbool.h>
#include <stdint.h>
#include "linked_list.h"

typedef enum {
  PATH_RELATIVE,
  PATH_PARENT,
  PATH_FILE
} path_type_t;

typedef struct {
  path_type_t type;
  char* value;
  bool is_last;
} path_token_t;

typedef struct {
  linked_list_t *token_types;
  char* name;
  bool is_absolute;
} path_parse_t;

path_parse_t *file_path_parse(char *path);

#endif //FILESYSTEM_FILE_PATH_H
