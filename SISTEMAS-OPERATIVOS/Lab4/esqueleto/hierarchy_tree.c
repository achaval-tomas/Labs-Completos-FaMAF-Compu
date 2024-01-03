/*
 * hierarchy_tree.h
 *
 * This data sctructure is designed to represent complex hierarchical structures
 * where each parent may have more than one children.
 * Stored values must be pointers.
 *
 * It's optimized for:
 *   - Fast access to items by their "name".
 *   - Fast traverse of a parent's direct childer in the original hierarchy
       (not the tree structure)
 * It's a standard binary search tree mixed with linked list structures using
 * the same nodes. The binary tree alows quick search of random elements,
 * and the linked list optimizes the iteration over all children of a given
 * parent.
 *
 * All members and functions that have `h_` in the name, reference the hierarchy
 * and not the binary search tree structure. For example, root->parent meas
 * parent in the search tree, while root->h_parent means parent on the
 * hierarchy. Note that a child on the binary tree does not imply being a child
 * on the hierarchy. However, all hierarchical childs will be inside subtrees
 * in the binary tree estructure.
 */

#include "hierarchy_tree.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/******************** NODE DATA STRUCTURE ********************/

typedef struct h_tree_s *h_tree;

struct h_tree_s {
    void *data;            // node will store an integer
    h_tree right;          // right child
    h_tree left;           // left child
    h_tree h_parent;       // Pointer to parent (in hierarchy)
    h_tree h_next_sibling; // Pointer to following children of h_parent.
    h_tree h_children;     // Pointer to first child (in hierarchy).
    int size;              // number of elements in subtrees + 1
};

static h_tree h_node_init(void *new_data, h_tree h_parent) {
    h_tree new_node;
    new_node = malloc(sizeof(struct h_tree_s));
    new_node->data = new_data;
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->size = 1;
    new_node->h_parent = h_parent; // May be null
    if (h_parent != NULL) {
        // Add new node as new head of h_parent's children list
        new_node->h_next_sibling = h_parent->h_children;
        h_parent->h_children = new_node;
    } else {
        new_node->h_next_sibling = NULL;
    }
    new_node->h_children = NULL;
    return new_node;
}

void h_tree_destroy(h_tree root, data_modify_fn data_destroy) {
    if (root == NULL)
        return;
    data_destroy(root->data);
    if (root->right != NULL) {
        h_tree_destroy(root->right, data_destroy);
    }
    if (root->left != NULL) {
        h_tree_destroy(root->left, data_destroy);
    }
    free(root);
}

/***************** ACCESORS *****************/
int h_tree_size(const h_tree root) {
    if (root == NULL) {
        return 0;
    }
    return root->size;
}

void *h_tree_get_data(const h_tree root) {
    if (root == NULL) {
        errno = EINVAL;
        return NULL;
    }
    return root->data;
}

h_tree h_tree_get_h_parent(const h_tree root) {
    if (root == NULL) {
        return NULL;
    }
    return root->h_parent;
}

/* Returs a pointer to the node whose value is equal to @key */
h_tree h_tree_search(const h_tree root, const void *key,
                     data_cmp_fn data_cmp_key) {
    if (key == NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (root == NULL)
        return NULL; // key not found or empty tree
    // search for the key
    int smaller_data = data_cmp_key(root->data, key);
    if (smaller_data < 0) {
        return h_tree_search(root->right, key, data_cmp_key);
    }
    if (smaller_data > 0) {
        return h_tree_search(root->left, key, data_cmp_key);
    }
    return root; // it's this one!
}

/***************** MODIFIERS *****************/
static inline void update_size(h_tree root) {
    root->size = 1 + h_tree_size(root->left) + h_tree_size(root->right);
}

h_tree h_tree_insert(h_tree root, void *new_data, h_tree h_parent,
                     data_cmp_fn data_cmp) {
    if (new_data == NULL) {
        errno = EINVAL;
        return root;
    }
    if (root == NULL) {
        h_tree new_node = h_node_init(new_data, h_parent);
        return new_node;
    }

    int cmp = data_cmp(new_data, root->data);
    if (cmp > 0) { // x is greater. Should be inserted to right
        root->right = h_tree_insert(root->right, new_data, h_parent, data_cmp);
    } else if (cmp < 0) { // x is smaller should be inserted to left
        root->left = h_tree_insert(root->left, new_data, h_parent, data_cmp);
    }
    update_size(root);
    return root;
}

static inline bool is_minimum(const h_tree root) { return root->left == NULL; }

static void swap_nodes(h_tree old_file, h_tree new_file) {
    h_tree temp_old_l = NULL, temp_old_r = NULL;
    temp_old_l = old_file->left;
    temp_old_r = old_file->right;
    old_file->left = new_file->left;
    old_file->right = new_file->right;
    new_file->left = temp_old_l;
    new_file->right = temp_old_r;
    update_size(new_file);
    update_size(old_file);
}

/* Remove a child node from the hierarchy. This does not affect tree structure.
 * Child parent is set to NULL
 */
static void remove_child_from_h(h_tree h_child) {
    if (h_child == NULL || h_child->h_parent == NULL) {
        return;
    }
    h_tree child_iterator = h_child->h_parent->h_children;
    if (child_iterator == NULL) { // Something went wrong
        errno = EINVAL;
        return;
    }
    if (child_iterator == h_child) {
        // It's the first child of parent's chain
        h_child->h_parent->h_children = h_child->h_next_sibling;
        return;
    }
    while (child_iterator->h_next_sibling != h_child &&
           child_iterator->h_next_sibling != NULL) {
        child_iterator = child_iterator->h_next_sibling;
    }
    child_iterator->h_next_sibling = h_child->h_next_sibling;
}

static h_tree delete_root(h_tree root, data_modify_fn data_destroy) {
    h_tree minimum = NULL;
    if (root->data != NULL) {
        data_destroy(root->data);
    }
    if (root->left == NULL || root->right == NULL) { // One or no Child
        if (root->left == NULL) {
            minimum = root->right;
        } else {
            minimum = root->left;
        }
        free(root);
        return minimum;
    }
    // Two Children. Copy min child data reference of right tree to current root
    h_tree minimum_parent = root->right;
    if (is_minimum(minimum_parent)) {
        minimum_parent->left = root->left;
        update_size(minimum_parent);
        free(root);
        return minimum_parent;
    }
    minimum = minimum_parent->left; // We are sure it's not null
    minimum_parent->size--;         // We will eventually remove one child
    while (!is_minimum(minimum)) {
        minimum = minimum->left;
        minimum_parent = minimum;
        minimum_parent->size--; // We will eventually remove one child
    }
    swap_nodes(root, minimum);
    minimum_parent->left = delete_root(root, data_destroy);
    return minimum;
}

// funnction to h_delete_node_tree a h_tree
h_tree h_tree_delete(h_tree root, const void *key, data_cmp_fn data_cmp_key,
                     data_modify_fn data_destroy) {
    if (key == NULL || root == NULL) {
        return root; // Key not present
    }
    // search for the key to be deleted. Is it the root?
    int cmp = data_cmp_key(root->data, key);
    if (cmp == 0) {
        // update hierarchy
        remove_child_from_h(root);
        return delete_root(root, data_destroy);
    }
    if (cmp < 0) {
        root->right =
            h_tree_delete(root->right, key, data_cmp_key, data_destroy);
    } else if (cmp > 0) {
        root->left = h_tree_delete(root->left, key, data_cmp_key, data_destroy);
    }
    // If key is not present in the tree, nothing changes.
    update_size(root);
    return root;
}

/************* ITERATORS ********************/

void h_tree_iterate_preorder(h_tree root, data_modify_fn mod_fn) {
    if (root != NULL) {
        mod_fn(root->data);                           // visiting data at root
        h_tree_iterate_preorder(root->left, mod_fn);  // visiting left child
        h_tree_iterate_preorder(root->right, mod_fn); // visiting right child
    }
}

void h_tree_iterate_h_ancestors(h_tree root, data_modify_fn mod_fn) {
    h_tree h_parent = root;
    while (h_parent != NULL) {
        mod_fn(h_parent->data); // visiting data at h_parent
        h_parent = h_parent->h_parent;
    }
}

void h_tree_flatten_preorder(const h_tree root, void **elem_array) {
    if (root != NULL) {
        elem_array[0] = root->data;
        elem_array++;
        h_tree_flatten_preorder(root->left, elem_array);
        elem_array += h_tree_size(root->left);
        h_tree_flatten_preorder(root->right, elem_array);
    }
}

void h_tree_flatten_h_children(const h_tree root, void **elem_array) {
    if (root == NULL) {
        return;
    }
    h_tree child_it = root->h_children;
    while (child_it != NULL) {
        elem_array[0] = child_it->data;
        child_it = child_it->h_next_sibling;
        elem_array++;
    }
    elem_array[0] = NULL;
}
