/*
 * fat_fs_tree.h
 *
 * This data sctructure is an abstraction of a file system directory tree.
 * Stored values must be fat_file.
 *
 * Internally, it contains a h_tree. It functions as a wrapper, setting the
 * correct compare, modify and destroy functions for fat_files.
 */

#ifndef FAT_FS_TREE_H
#define FAT_FS_TREE_H

#ifndef REPLACE_MOCKS
#include "fat_file.h"
#endif

#ifdef REPLACE_MOCKS
#include "mock_fat_file.h"
#endif

#include "hierarchy_tree.h"
#include <stdlib.h>

typedef struct fat_tree_s *fat_tree;
typedef h_tree fat_tree_node;

/* Initializes a new empty tree */
fat_tree fat_tree_init();

/* Destroys every node in @tree, frees the memory of all it's fat_files */
void fat_tree_destroy(fat_tree tree);

/* Returns the number of files in the tree */
int fat_tree_size(const fat_tree tree);

/* Inserts @new_file into @tree. @parent should be a reference to the file's
 * directory, and must be a node of @tree. It may be NULL.
 * The TAD is the owner of the reference to @new_file and will destroy it when
 * destroying the whole tree.
 */
fat_tree fat_tree_insert(fat_tree tree, fat_tree_node parent,
                         const fat_file new_file);

/* Returns a reference to fat_file with filepath @key. If @key is not found,
 * returns NULL. If @key or @tree are NULL, sets errno to EINVAL.
 * The TAD is still the owner of the reference. Modifying it's memory will
 * modify the content of the tree.
 */
fat_file fat_tree_search(const fat_tree tree, const char *key);

/* NOTE: The following two functions act as wrappers for the h_tree functions,
 * to avoid coupling the code too much.
 * These search functions can be used to get a copy of the h_tree node, to
 * optimize functions that need to travel the tree from a certain node, avoiding
 * innecesary double searches.
 */

/* Returns a reference to the node (of the internal tree) that matches @key
 * If @tree is NULL, @tree is empty or @key is NULL, returns NULL. If key
 * is NULL, sets errno to EINVAL*/
fat_tree_node fat_tree_node_search(const fat_tree tree, const char *key);

/* Extract the file pointer from a fat_tree_node @node structure.
 * If @node is NULL, returns NULL and sets errno to EINVAL.
 */
fat_file fat_tree_get_file(const fat_tree_node node);

/* Returns a reference to the directory that contains the fat_file in @node.
 * It @node is NULL or has no parent, returns NULL.
 */
fat_file fat_tree_get_parent(const fat_tree_node node);

/* Applies function fat_file_inc_num_times_opened to the fat_file in @node
 * and all it's ancestor directories.
 */
void fat_tree_inc_num_times_opened(fat_tree_node node);

/* Applies function fat_file_dec_num_times_opened to the fat_file in @node
 * and all it's ancestor directories.
 */
void fat_tree_dec_num_times_opened(fat_tree_node node);

/* Deletes the fat_file with filepath @key from @tree. The fat_file will be
 * destroyed by this action. If the file is not found, no action is taken.
 * If tree is NULL, errno is set to EINVAL.
 */
fat_tree fat_tree_delete(fat_tree tree, const char *key);

/* Applies the function @mod_fn to all elements in @tree, in pre-order. */
void fat_tree_iterate_preorder(fat_tree tree, data_modify_fn mod_fn);

/* Prints the filepaths of the files in @tree, in preorder */
void fat_tree_print_preorder(fat_tree tree);

/* Returns an array with references all nodes in @tree.
 * The array has fat_tree_size(@tree) elements. References still belong to the
 * tree, do NOT modify them. Caller owns the array and must free its memory.
 */
void **fat_tree_flatten_preorder(const fat_tree tree);

/* Returns an array with references to child nodes of @dir_node in the
 * hierarchy. The array has fat_tree_get_file(@dir_node)->dir.nentries + 1
 * elements and it's NULL terminated (in case there are less children than
 * entries). References still belong to the tree, do NOT modify them.
 * Caller must ensure @dir_node is a directory.
 * Caller owns the array and must free its memory.
 */
fat_file *fat_tree_flatten_h_children(const fat_tree_node dir_node);

#endif /* FAT_FS_TREE_H */
