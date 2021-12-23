#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "filesystem.h"
#include "command_line_parser.h"

#define COMMAND_LINE_MAX_NUM_ARG 20
#define COMMAND_LINE_MAX_TEXT_SIZE 50
#define COMMAND_LINE_DELIM_WHITESPACE " "

typedef enum {
  CL_UNKNOWN,
  CL_INT,
  CL_STRING
} command_type_t;

typedef struct {
  command_type_t type;
  char *arg;
} command_arg_t;

typedef struct {
  command_arg_t **argv;
  int argc;
} command_line_t;

static command_line_t *command_line_new() {
  command_line_t *cl = malloc(sizeof(command_line_t));
  cl->argv = (command_arg_t **) malloc(COMMAND_LINE_MAX_NUM_ARG * sizeof(command_arg_t));
  cl->argc = 0;
  return cl;
}

static void command_line_free(command_line_t *cl) {
  cl->argc = 0;
}

inline static bool command_line_arg_is_int(command_line_t *cl, int index) {
  return cl->argv[index]->type == CL_INT;
}

inline static bool command_line_arg_is_str(command_line_t *cl, int index) {
  return cl->argv[index]->type == CL_STRING;
}

static bool command_line_check_arg(command_line_t *cl, int index) {
  return !cl->argc || index > cl->argc || index < 1;
}

static int command_line_arg_int(command_line_t *cl, int index) {
  if (command_line_check_arg(cl, index)) {
    return -1;
  }
  if (command_line_arg_is_int(cl, index)) {
    return atoi(cl->argv[index]->arg); // NOLINT
  }
  return -1;
}

static char *command_line_arg_str(command_line_t *cl, int index) {
  if (command_line_check_arg(cl, index)) {
    return NULL;
  }
  if (command_line_arg_is_str(cl, index)) {
    return cl->argv[index]->arg; // NOLINT
  }
  return NULL;
}

static int command_line_arg_is_number(const char *str) {
  for (int i = 0; i < strlen(str); i++) {
    if (!isdigit(str[i])) return 0;
  }
  return 1;
}

static void command_line_set_arg(command_line_t *cl, char *token, int index) {
  if (!token) return;
  cl->argv[index] = (command_arg_t *) malloc(sizeof(command_arg_t));
  if (command_line_arg_is_number(token)) {
    cl->argv[index]->type = CL_INT;
  } else {
    cl->argv[index]->type = CL_STRING;
  }
  cl->argv[index]->arg = token;
  cl->argc++;
}

static void command_line_read(command_line_t *cl) {
  char *value = malloc(sizeof(char *) * COMMAND_LINE_MAX_TEXT_SIZE);
  printf("Enter command: \n");

  fgets(value, COMMAND_LINE_MAX_TEXT_SIZE, stdin);
  size_t string_len = strlen(value);
  size_t last_element = string_len - 1;
  if (string_len > 0 && (value[last_element] == '\n')) {
    value[last_element] = '\0';
  }

  char *token = strtok(value, COMMAND_LINE_DELIM_WHITESPACE); // NOLINT
  command_line_set_arg(cl, token, 0);

  for (int i = 1; token; ++i) {
    token = strtok(NULL, COMMAND_LINE_DELIM_WHITESPACE); // NOLINT
    command_line_set_arg(cl, token, i);
  }
}

inline static bool command_line_check(command_line_t *cl, const char *command, int argc) {
  if (cl->argv[0]->type != CL_STRING || cl->argc - 1 != argc) {
    return false;
  }
  return strcmp(cl->argv[0]->arg, command) == 0;
}

#define COMMAND_LINE_IS_EXIT(cl)      command_line_check((cl), "exit", 0)
#define COMMAND_LINE_IS_LS(cl)        command_line_check((cl), "ls", 0)
#define COMMAND_LINE_IS_MOUNT(cl)     command_line_check((cl), "mount", 0)
#define COMMAND_LINE_IS_UNMOUNT(cl)   command_line_check((cl), "unmount", 0)

#define COMMAND_LINE_IS_MKFS(cl)      command_line_check((cl), "mkfs", 1) && command_line_arg_is_int(cl, 1)
#define COMMAND_LINE_IS_FSTAT(cl)     command_line_check((cl), "fstat", 1) && command_line_arg_is_int(cl, 1)
#define COMMAND_LINE_IS_CREATE(cl)    command_line_check((cl), "create", 1) && command_line_arg_is_str(cl, 1)
#define COMMAND_LINE_IS_UNLINK(cl)    command_line_check((cl), "unlink", 1) && command_line_arg_is_str(cl, 1)
#define COMMAND_LINE_IS_OPEN(cl)      command_line_check((cl), "open", 1) && command_line_arg_is_str(cl, 1)
#define COMMAND_LINE_IS_CLOSE(cl)     command_line_check((cl), "close", 1) && command_line_arg_is_int(cl, 1)
#define COMMAND_LINE_IS_CD(cl)        command_line_check((cl), "cd", 1) && command_line_arg_is_str(cl, 1)
#define COMMAND_LINE_IS_MKDIR(cl)     command_line_check((cl), "mkdir", 1) && command_line_arg_is_str(cl, 1)
#define COMMAND_LINE_IS_RMDIR(cl)     command_line_check((cl), "rmdir", 1) && command_line_arg_is_str(cl, 1)

#define COMMAND_LINE_IS_LINK(cl)          \
  command_line_check((cl), "link", 2) &&  \
  command_line_arg_is_str(cl, 1)      &&  \
  command_line_arg_is_str(cl, 2)

#define COMMAND_LINE_IS_SYMLINK(cl)          \
  command_line_check((cl), "symlink", 2) &&  \
  command_line_arg_is_str(cl, 1)      &&  \
  command_line_arg_is_str(cl, 2)

#define COMMAND_LINE_IS_TRUNCATE(cl)         \
  command_line_check((cl), "truncate", 2) && \
  command_line_arg_is_str(cl, 1)          && \
  command_line_arg_is_int(cl, 2)

#define COMMAND_LINE_IS_READ(cl)          \
  command_line_check((cl), "read", 3) &&  \
  command_line_arg_is_int(cl, 1)      &&  \
  command_line_arg_is_int(cl, 2)      &&  \
  command_line_arg_is_int(cl, 3)

#define COMMAND_LINE_IS_WRITE(cl)          \
  command_line_check((cl), "write", 3) &&  \
  command_line_arg_is_int(cl, 1)       &&  \
  command_line_arg_is_int(cl, 2)       &&  \
  command_line_arg_is_int(cl, 3)

static void command_line_execute(command_line_t *cl) {
  command_line_read(cl);
  if (COMMAND_LINE_IS_EXIT(cl)) {
    free(cl);
    exit(EXIT_SUCCESS);
  }
  if (COMMAND_LINE_IS_MKFS(cl)) {
    fs_mkfs(command_line_arg_int(cl, 1));
    return;
  }
  if (COMMAND_LINE_IS_MOUNT(cl)) {
    fs_mount();
    return;
  }
  if (COMMAND_LINE_IS_UNMOUNT(cl)) {
    fs_unmount();
    return;
  }
  if (COMMAND_LINE_IS_FSTAT(cl)) {
    fs_fstat(command_line_arg_int(cl, 1));
    return;
  }
  if (COMMAND_LINE_IS_LS(cl)) {
    fs_ls();
    return;
  }
  if (COMMAND_LINE_IS_CREATE(cl)) {
    fs_create(command_line_arg_str(cl, 1));
    return;
  }
  if (COMMAND_LINE_IS_OPEN(cl)) {
    fs_open(command_line_arg_str(cl, 1));
    return;
  }
  if (COMMAND_LINE_IS_CLOSE(cl)) {
    fs_close(command_line_arg_int(cl, 1));
    return;
  }
  if (COMMAND_LINE_IS_READ(cl)) {
    int fd = command_line_arg_int(cl, 1);
    int offset = command_line_arg_int(cl, 2);
    int size = command_line_arg_int(cl, 3);
    fs_read(fd, offset, size);
    return;
  }
  if (COMMAND_LINE_IS_WRITE(cl)) {
    int fd = command_line_arg_int(cl, 1);
    int offset = command_line_arg_int(cl, 2);
    int size = command_line_arg_int(cl, 3);
    char *value = malloc(sizeof(char) * size);
    printf("Enter text to write: \n");
    fgets(value, COMMAND_LINE_MAX_TEXT_SIZE, stdin);

    fs_write(fd, value, offset, size);
    return;
  }
  if (COMMAND_LINE_IS_LINK(cl)) {
    char *path1 = command_line_arg_str(cl, 1);
    char *path2 = command_line_arg_str(cl, 2);
    fs_link(path1, path2);
    return;
  }
  if (COMMAND_LINE_IS_UNLINK(cl)) {
    char *path1 = command_line_arg_str(cl, 1);
    fs_unlink(path1);
    return;
  }
  if (COMMAND_LINE_IS_TRUNCATE(cl)) {
    char *path = command_line_arg_str(cl, 1);
    uint32_t size = command_line_arg_int(cl, 2);
    fs_truncate(path, size);
    return;
  }
  if (COMMAND_LINE_IS_CD(cl)) {
    char *path = command_line_arg_str(cl, 1);
    fs_cd(path);
    return;
  }
  if (COMMAND_LINE_IS_MKDIR(cl)) {
    char *path = command_line_arg_str(cl, 1);
    fs_mkdir(path);
    return;
  }
  if (COMMAND_LINE_IS_RMDIR(cl)) {
    char *path = command_line_arg_str(cl, 1);
    fs_rmdir(path);
    return;
  }
  if (COMMAND_LINE_IS_SYMLINK(cl)) {
    char *str = command_line_arg_str(cl, 1);
    char *path = command_line_arg_str(cl, 2);
    fs_symlink(str, path);
    return;
  }
}

_Noreturn void command_line_run() {
  command_line_t *cl = command_line_new();
  while (true) {
    command_line_execute(cl);
    command_line_free(cl);
  }
}

