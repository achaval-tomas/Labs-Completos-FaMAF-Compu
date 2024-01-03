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

#ifndef HIERARCHY_TREE_H
#define HIERARCHY_TREE_H

#include <stdlib.h>

// Function to compare two h_node_tree->data in the tree
typedef int (*data_cmp_fn)(const void *d1, const void *d2);
typedef void (*data_modify_fn)(void *d1);

typedef struct h_tree_s *h_tree;

/* Destroys every node in @tree, and applies @data_destroy function to the
 * data field.
 */
void h_tree_destroy(h_tree tree, data_modify_fn data_destroy);

/* Returns the number of elements of the left and right subtrees + 1 */
int h_tree_size(const h_tree tree);

/* Returns a reference to the data stored in the root of the tree @root.
 * The TAD is still the owner of the reference. Modifying it's memory will
 * modify the content of the tree.
 */
void *h_tree_get_data(const h_tree root);

/* Returns a reference to the node containing @root's parent in the hierarchy.
 * The TAD is still the owner of the reference. Modifying it's memory will
 * modify the content of the tree.
 */
h_tree h_tree_get_h_parent(const h_tree root);

/* Returns a reference to the node in @root that matches @key accoding to
 * @data_cmp_key. I.e, data_cmp_key(node->data, key) == 0
 * If @key is not found, returns NULL. If @key is NULL, sets errno to EINVAL.
 * The TAD is still the owner of the reference. Modifying it's memory will
 * modify the content of the tree.
 */
h_tree h_tree_search(const h_tree root, const void *key,
                     data_cmp_fn data_cmp_key);

/* Inserts @new_data into @tree using the funcition @data_cmp to determine the
 * location of the new node in the tree. To correctly retrieve the node, the
 * same @data_cmp function must be used when calling h_tree_search and
 * h_tree_delete.
 * @h_parent should be a reference to the parent of @new_data in the hierarchy,
 * and must be a node of @tree. It may be NULL.
 * The TAD is the owner of the reference to @new_data and will destroy it when
 * destroying the whole tree.
 */
h_tree h_tree_insert(h_tree tree, void *new_data, h_tree h_parent,
                     data_cmp_fn data_cmp);

/* Deletes @key from @tree using the funcition @data_cmp to determine the
 * location of the node in the tree. The function @data_destroy will be applied
 * to the node when found.
 * In case of error, errno is set to EINVAL.
 */
h_tree h_tree_delete(h_tree tree, const void *key, data_cmp_fn data_cmp_key,
                     data_modify_fn data_destroy);

/* Applies the function @mod_fn to all elements in @tree, in pre-order. */
void h_tree_iterate_preorder(h_tree tree, data_modify_fn mod_fn);

/* Applies function @mod_fn to @tree and all its ancestors in the hierarchy (not
 * tree structure).
 */
void h_tree_iterate_h_ancestors(h_tree tree, data_modify_fn mod_fn);

/* Fills up @elem_array with references to the tree data. References still
 * belong to the tree, do NOT modify them.
 * Caller must ensure @elem_array can hold all nodes (fat_tree_size(@tree)).
 * Caller owns the array and must free its memory.
 */
void h_tree_flatten_preorder(const h_tree tree, void **elem_array);

/* Fills up @elem_array with references to direct h_children of root. Fills
 * last position with NULL terminator.
 * References still belong to the tree, do NOT modify them.
 * Caller must ensure @elem_array can hold all h_children.
 * Caller owns the array and must free its memory.
 */
void h_tree_flatten_h_children(const h_tree root, void **elem_array);

#endif /* HIERARCHY_TREE_H */
