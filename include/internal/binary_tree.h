#ifndef FILESYSTEM_BINARY_TREE_H
#define FILESYSTEM_BINARY_TREE_H

typedef struct tree_node {
  char *name;
  uint32_t value;
  uint32_t index;
  uint32_t num_reserved_bits;
  struct tree_node *left;
  struct tree_node *right;
} tree_node_t;

tree_node_t *tree_node_new(uint32_t value, uint32_t index, uint32_t num_reserved_bits);

typedef struct {
  tree_node_t *ptr;
} tree_t;

tree_t *tree_new();

void tree_insert_node_value(tree_node_t **link, uint32_t value);

void tree_insert_node(tree_node_t **link, tree_node_t *tree_node);

tree_node_t *tree_find_node(tree_node_t *link, uint32_t value);

tree_node_t *tree_delete_smallest_node(tree_node_t **link);

void tree_delete_node(tree_node_t **link, uint32_t value);

void tree_delete_all(tree_node_t **link);

uint32_t tree_get_height(tree_node_t *link);

tree_node_t *tree_find_node_by_name(tree_node_t *link, const char *value);

void tree_show(tree_node_t *link);

#endif // FILESYSTEM_BINARY_TREE_H
