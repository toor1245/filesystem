#include "file_path.h"
#include <stdlib.h>
#include <string.h>

#define FILE_PATH_DELIM_SLASH  "/"

#define FILE_PATH_CURRENT "."
#define FILE_PATH_PARENT  ".."
#define FILE_PATH_ROOT    "root"

static path_type_t path_type_parse(const char *token) {
  if (strcmp(token, FILE_PATH_CURRENT) == 0) {
    return PATH_RELATIVE;
  }
  if (strcmp(token, FILE_PATH_PARENT) == 0) {
    return PATH_PARENT;
  }
  return PATH_FILE;
}

static path_token_t *path_token_new(char *token) {
  path_token_t *path_token = malloc(sizeof(path_token_t));
  path_token->type = path_type_parse(token);
  path_token->value = token;
  path_token->is_last = false;
  return path_token;
}

static path_parse_t *path_parse_new() {
  path_parse_t *path_parse = malloc(sizeof(path_parse_t));
  path_parse->token_types = linked_list_new();
  path_parse->is_absolute = false;
  path_parse->name = NULL;
  return path_parse;
}


path_parse_t *file_path_parse(char *path) {
  if (!path) {
    return NULL;
  }
  size_t path_len = strlen(path);
  char *buffer = malloc(sizeof(char) * path_len);
  strcpy(buffer, path);
  char *token = strtok(buffer, FILE_PATH_DELIM_SLASH); // NOLINT
  if (token == NULL) {
    return NULL;
  }

  path_parse_t *path_parse = path_parse_new();
  if (strcmp(token, FILE_PATH_ROOT) == 0) {
    path_parse->is_absolute = true;
  } else {
    path_token_t *path_token = path_token_new(token);
    if (path_token->type == PATH_FILE) {
      return NULL;
    }
    linked_list_push(path_parse->token_types, (void *) path_token);
  }

  while (token) {
    token = strtok(NULL, FILE_PATH_DELIM_SLASH); // NOLINT
    if (token == NULL) {
      path_token_t *path_token = (path_token_t *) path_parse->token_types->tail->value;
      path_token->is_last = true;
      break;
    }
    path_parse->name = token;
    linked_list_push(path_parse->token_types, (void *) path_token_new(token));
  }
  return path_parse;
}
