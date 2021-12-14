#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "binary_tree.h"

void tree_insert_node_value(tree_node_t **link, uint32_t value) {
  if (!*link) {
    (*link) = tree_node_new(value, 0, 0);
  } else if ((value) < ((*(*link)).value)) {
    tree_insert_node_value(&((*(*link)).left), value);
  } else {
    tree_insert_node_value(&((*(*link)).right), value);
  }
}

void tree_insert_node(tree_node_t **link, tree_node_t *tree_node) {
  if (!*link) {
    (*link) = (tree_node_t *) malloc(sizeof(tree_node_t));
    (*(*link)).value = (*tree_node).value;
    (*(*link)).index = (*tree_node).index;
    (*(*link)).num_reserved_bits = (*tree_node).num_reserved_bits;
    (*(*link)).left = NULL;
    (*(*link)).right = NULL;
  } else if ((*tree_node).value < (*(*link)).value) {
    tree_insert_node(&((*(*link)).left), tree_node);
  } else {
    tree_insert_node(&((*(*link)).right), tree_node);
  }
}

tree_node_t *tree_find_node(tree_node_t *link, uint32_t value) {
  if (!link)
    return NULL;
  if (((*link).value) == value) {
    return (link);
  } else if (value >= ((*link).value)) {
    return (tree_find_node((*link).right, value));
  } else {
    return (tree_find_node((*link).left, value));
  }
}

tree_node_t *tree_delete_smallest_node(tree_node_t **link) {
  if ((*(*link)).left) {
    return (tree_delete_smallest_node(&((*(*link)).left)));
  } else {
    tree_node_t *temp;
    temp = *link;
    (*link) = (*(*link)).right;
    return temp;
  }
}

void tree_delete_node(tree_node_t **link, uint32_t value) {
  if (!(*link)) return;
  if (value < (*(*link)).value) {
    tree_delete_node(&((*(*link)).left), value);
  } else if (value > (*(*link)).value) {
    tree_delete_node(&((*(*link)).right), value);
  } else {
    tree_node_t *temp = *link;
    if ((*(*link)).right == NULL) {
      (*link) = (*(*link)).left;
    } else if ((*(*link)).left == NULL) {
      (*link) = (*(*link)).right;
    } else {
      temp = tree_delete_smallest_node(&((*(*link)).right));
    }
    (*(*link)).value = (*temp).value;
    free(temp);
  }
}

void tree_delete_all(tree_node_t **link) {
  if (!(*link)) return;
  tree_delete_all(&((*(*link)).left));
  tree_delete_all(&((*(*link)).right));
  free((*link));
  *link = NULL;
}

uint32_t tree_get_height(tree_node_t *link) {
  if (!link) return -1;
  uint32_t u = tree_get_height((*link).left);
  uint32_t v = tree_get_height((*link).right);
  return u > v ? u + 1 : v + 1;
}

tree_t *tree_new() {
  tree_t *tree = malloc(sizeof(tree_t));
  tree->ptr = NULL;
  return tree;
}

tree_node_t *tree_node_new(uint32_t value, uint32_t index, uint32_t num_reserved_bits) {
  tree_node_t *tree_node = malloc(sizeof(tree_node_t));
  tree_node->index = index;
  tree_node->value = value;
  tree_node->num_reserved_bits = num_reserved_bits;
  tree_node->right = NULL;
  tree_node->left = NULL;
  return tree_node;
}

tree_node_t *tree_find_node_by_name(tree_node_t *link, const char *value) {
  if (!link) {
    return NULL;
  }

  if (strcmp((*link).name, value) == 0) {
    return link;
  }

  tree_node_t *left_tree_node = tree_find_node_by_name(link->left, value);
  if (left_tree_node) {
    return left_tree_node;
  }

  tree_node_t *right_tree_node = tree_find_node_by_name(link->right, value);
  if (right_tree_node) {
    return right_tree_node;
  }
  return NULL;
}


void tree_show(tree_node_t *link) {
  if (link == NULL) {
    return;
  }
  tree_show((*link).right);
  printf("file: %s\n", link->name);
  tree_show((*link).left);
}