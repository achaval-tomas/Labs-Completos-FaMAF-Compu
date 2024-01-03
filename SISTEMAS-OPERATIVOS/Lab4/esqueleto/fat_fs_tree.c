/*
 * fat_fs_tree.h
 *
 * This data sctructure is an abstraction of a file system directory tree.
 * Stored values must be fat_file.
 *
 * Internally, it contains a h_tree. It functions as a wrapper, setting the
 * correct compare, modify and destroy functions for fat_files.
 */

#include "fat_fs_tree.h"
#include "hierarchy_tree.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

struct fat_tree_s {
    h_tree file_tree;
    data_cmp_fn file_cmp;
    data_modify_fn file_destroy;
    data_cmp_fn file_cmp_key;
};

fat_tree fat_tree_init() {
    fat_tree new_tree = malloc(sizeof(struct fat_tree_s));
    new_tree->file_tree = NULL; // empty tree
    new_tree->file_cmp = (data_cmp_fn)fat_file_cmp;
    new_tree->file_destroy = (data_modify_fn)fat_file_destroy;
    new_tree->file_cmp_key = (data_cmp_fn)fat_file_cmp_path;
    return new_tree;
}

void fat_tree_destroy(fat_tree tree) {
    if (tree != NULL) {
        if (tree->file_tree != NULL) {
            h_tree_destroy(tree->file_tree, tree->file_destroy);
        }
        free(tree);
    }
    tree = NULL;
}

int fat_tree_size(const fat_tree tree) {
    if (tree == NULL) {
        return -1;
    }
    return h_tree_size(tree->file_tree);
}

fat_tree fat_tree_insert(fat_tree tree, fat_tree_node parent,
                         const fat_file new_file) {
    if (tree == NULL || new_file == NULL) {
        errno = EINVAL;
        return NULL;
    }
    tree->file_tree = h_tree_insert(tree->file_tree, (void *)new_file,
                                    (h_tree)parent, tree->file_cmp);
    return tree;
}

fat_file fat_tree_search(const fat_tree tree, const char *key) {
    if (tree == NULL || key == NULL || tree->file_tree == NULL) {
        return NULL;
    }
    return h_tree_get_data(fat_tree_node_search(tree, key));
}

fat_tree_node fat_tree_node_search(const fat_tree tree, const char *key) {
    return h_tree_search(tree->file_tree, (void *)key, tree->file_cmp_key);
}

fat_file fat_tree_get_file(const fat_tree_node node) {
    return h_tree_get_data(node);
}

fat_file fat_tree_get_parent(const fat_tree_node node) {
    return (fat_file)h_tree_get_data(h_tree_get_h_parent(node));
}

void fat_tree_inc_num_times_opened(fat_tree_node node) {
    h_tree_iterate_h_ancestors(node,
                               (data_modify_fn)fat_file_inc_num_times_opened);
}

void fat_tree_dec_num_times_opened(fat_tree_node node) {
    h_tree_iterate_h_ancestors(node,
                               (data_modify_fn)fat_file_dec_num_times_opened);
}

fat_tree fat_tree_delete(fat_tree tree, const char *key) {
    if (tree == NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (key == NULL || tree->file_tree == NULL) {
        return tree;
    }
    tree->file_tree = h_tree_delete(tree->file_tree, key, tree->file_cmp_key,
                                    tree->file_destroy);
    return tree;
}

void fat_tree_iterate_preorder(fat_tree tree, data_modify_fn mod_fn) {
    if (tree != NULL) {
        h_tree_iterate_preorder(tree->file_tree, mod_fn);
    }
}

// DEBUGGING FUNCTIONS
static void print_node(void *file_data) {
    fat_file file = (fat_file)file_data;
    printf("%s ", file->filepath);
}

void fat_tree_print_preorder(fat_tree tree) {
    if (tree != NULL) {
        h_tree_iterate_preorder(tree->file_tree, (data_modify_fn)print_node);
    }
    printf("\n");
}

void **fat_tree_flatten_preorder(const fat_tree tree) {
    if (tree == NULL || h_tree_size(tree->file_tree) == 0) {
        return NULL;
    }
    void **elem_array = calloc(h_tree_size(tree->file_tree), sizeof(void *));
    h_tree_flatten_preorder(tree->file_tree, elem_array);
    return elem_array;
}

fat_file *fat_tree_flatten_h_children(const fat_tree_node dir_node) {
    if (dir_node == NULL || h_tree_size(dir_node) == 0) {
        return NULL;
    }
    int tree_size = fat_tree_get_file(dir_node)->dir.nentries;
    fat_file *elem_array = calloc(tree_size + 1, sizeof(fat_file));
    h_tree_flatten_h_children(dir_node, (void **)elem_array);
    return elem_array;
}