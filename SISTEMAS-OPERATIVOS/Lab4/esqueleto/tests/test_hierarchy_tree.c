/*
 * Tests for the h_tree data sctructure
 *
 */

#include "hierarchy_tree.h"
#include <assert.h>
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ELEMS 100

char *test_elems[7] = {"5", "6", "3", "7", "2", "4", "1"};

static h_tree add_elem_list(h_tree tree, int size, char **elem_array) {
    void *new_elem = NULL;
    int current_size = h_tree_size(tree);
    for (int i = 0; i < size; ++i) {
        new_elem = (void *)strdup(elem_array[i]);
        tree = h_tree_insert(tree, new_elem, NULL, (data_cmp_fn)strcmp);
        fail_unless(h_tree_size(tree) == current_size + i + 1);
    }
    return tree;
}

static h_tree add_test_elems(h_tree tree) {
    return add_elem_list(tree, 7, test_elems);
}

static void compare_preorder_list(h_tree tree, char **expected_elems) {
    int size = h_tree_size(tree);
    void **tree_elems = malloc(h_tree_size(tree) * sizeof(void *));
    h_tree_flatten_preorder(tree, tree_elems);
    for (int i = 0; i < size; ++i) {
        fail_unless(strcmp(expected_elems[i], (char *)tree_elems[i]) == 0);
    }
    free(tree_elems);
}

// Used to iterate over tree and mark nodes that are visited setting the first
// element as X. Caller must ensure all nodes have at least one byte of memory
// allocated.
static void mark_node_visited(char *data) { *data = 'X'; }

h_tree tree = NULL;

START_TEST(test_init_destroy) {
    fail_unless(h_tree_size(tree) == 0);
    h_tree_destroy(tree, free);
    tree = NULL;
}
END_TEST

START_TEST(test_destroy_null) { h_tree_destroy(NULL, free); }
END_TEST

START_TEST(test_insert_null) {
    void *new_tree = NULL;
    new_tree = h_tree_insert(tree, NULL, NULL, (data_cmp_fn)strcmp);
    fail_unless(h_tree_size(new_tree) == 0);
    fail_unless(new_tree == tree);
    fail_unless(errno == EINVAL);
}
END_TEST

START_TEST(test_insert_null_tree) {
    tree = h_tree_insert(NULL, strdup("node 1"), NULL, (data_cmp_fn)strcmp);
    fail_unless(h_tree_size(tree) == 1);
    fail_unless(strcmp((char *)h_tree_get_data(tree), "node 1") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_insert_many) {
    tree = add_test_elems(tree);
    void **elem_array = calloc(h_tree_size(tree), sizeof(void *));
    h_tree_flatten_preorder(tree, elem_array);
    char *expected_elems[7] = {"5", "3", "2", "1", "4", "6", "7"};
    compare_preorder_list(tree, expected_elems);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_search_null_tree) {
    h_tree found_node = h_tree_search(NULL, "1", (data_cmp_fn)strcmp);
    fail_unless(found_node == NULL);
}
END_TEST

START_TEST(test_search_null_key) {
    h_tree found_node = h_tree_search(NULL, NULL, (data_cmp_fn)strcmp);
    fail_unless(errno == EINVAL);
    fail_unless(found_node == NULL);
}
END_TEST

START_TEST(test_search_small_tree) {
    h_tree found_node = NULL;
    tree = h_tree_insert(NULL, strdup("1"), NULL, (data_cmp_fn)strcmp);
    found_node = h_tree_search(tree, "1", (data_cmp_fn)strcmp);
    fail_unless(found_node != NULL);
    fail_unless(strcmp((char *)h_tree_get_data(found_node), "1") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_search_big_tree) {
    h_tree found_node = NULL;
    tree = add_test_elems(tree);
    found_node = h_tree_search(tree, "2", (data_cmp_fn)strcmp);
    fail_unless(found_node != NULL);
    fail_unless(strcmp((char *)h_tree_get_data(found_node), "2") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_search_non_existing_key) {
    h_tree found_node = NULL;
    tree = add_test_elems(tree);
    found_node = h_tree_search(tree, "0", (data_cmp_fn)strcmp);
    fail_unless(found_node == NULL);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_delete_leaf) {
    char *test_elems_no4[6] = {"5", "3", "2", "1", "6", "7"};
    tree = add_test_elems(tree);
    // tree_print_preorder(tree);
    tree = h_tree_delete(tree, "4", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 6);
    compare_preorder_list(tree, test_elems_no4);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_delete_middle) {
    char *test_elems_no3[6] = {"5", "4", "2", "1", "6", "7"};
    tree = add_test_elems(tree);
    tree = h_tree_delete(tree, "3", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 6);
    compare_preorder_list(tree, test_elems_no3);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_delete_root) {
    char *test_elems_no5[6] = {"6", "3", "2", "1", "4", "7"};
    tree = add_test_elems(tree);
    // tree_print_preorder(tree);
    tree = h_tree_delete(tree, "5", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 6);
    compare_preorder_list(tree, test_elems_no5);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_delete_all) {
    tree = add_test_elems(tree);
    tree = h_tree_delete(tree, "5", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 6);
    tree = h_tree_delete(tree, "1", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 5);
    tree = h_tree_delete(tree, "2", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 4);
    tree = h_tree_delete(tree, "7", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 3);
    tree = h_tree_delete(tree, "4", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 2);
    tree = h_tree_delete(tree, "6", (data_cmp_fn)strcmp, free);
    fail_unless(h_tree_size(tree) == 1);
    char *expected_elems[1] = {"3"};
    compare_preorder_list(tree, expected_elems);
    h_tree_destroy(tree, free);
}
END_TEST

/* Building the test suites */

Suite *binary_search_tree_suite(void) {
    Suite *test_suit = suite_create("binary_search_tree");
    TCase *tcase_functionality = tcase_create("Binary Search Tree functions");
    tcase_add_test(tcase_functionality, test_init_destroy);
    tcase_add_test(tcase_functionality, test_destroy_null);
    tcase_add_test(tcase_functionality, test_insert_null);
    tcase_add_test(tcase_functionality, test_insert_null_tree);
    tcase_add_test(tcase_functionality, test_insert_many);
    tcase_add_test(tcase_functionality, test_search_null_tree);
    tcase_add_test(tcase_functionality, test_search_null_key);
    tcase_add_test(tcase_functionality, test_search_small_tree);
    tcase_add_test(tcase_functionality, test_search_big_tree);
    tcase_add_test(tcase_functionality, test_search_non_existing_key);
    tcase_add_test(tcase_functionality, test_delete_leaf);
    tcase_add_test(tcase_functionality, test_delete_middle);
    tcase_add_test(tcase_functionality, test_delete_root);
    tcase_add_test(tcase_functionality, test_delete_all);
    suite_add_tcase(test_suit, tcase_functionality);

    return test_suit;
}

START_TEST(test_insert_with_parent) {
    h_tree child_node = NULL;
    tree = h_tree_insert(tree, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    fail_unless(h_tree_size(tree) == 1);
    tree = h_tree_insert(tree, strdup("l1.1"), tree, (data_cmp_fn)strcmp);
    fail_unless(h_tree_size(tree) == 2);
    child_node = h_tree_search(tree, "l1.1", (data_cmp_fn)strcmp);
    fail_unless(h_tree_get_h_parent(child_node) == tree);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_iterate_ancestors_null) {
    h_tree_iterate_h_ancestors(NULL, (data_modify_fn)mark_node_visited);
    fail_unless(errno >= 0);
}
END_TEST

START_TEST(test_iterate_ancestors_small_tree) {
    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    fail_unless(tree != NULL);
    h_tree_iterate_h_ancestors(tree, (data_modify_fn)mark_node_visited);
    fail_unless(errno >= 0);
    fail_unless(strcmp((char *)h_tree_get_data(tree), "X1") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_iterate_ancestors_small_tree2) {
    h_tree node_l2 = NULL, node_l21 = NULL;
    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1"), tree, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l2"), NULL, (data_cmp_fn)strcmp);
    node_l2 = h_tree_search(tree, "l2", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l2.1"), node_l2, (data_cmp_fn)strcmp);
    node_l21 = h_tree_search(tree, "l2.1", (data_cmp_fn)strcmp);

    h_tree_iterate_h_ancestors(node_l21, (data_modify_fn)mark_node_visited);
    fail_unless(errno >= 0);

    fail_unless(strcmp((char *)h_tree_get_data(node_l21), "X2.1") == 0);
    fail_unless(strcmp((char *)h_tree_get_data(node_l2), "X2") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

/* Tree used for test. Ancestor iteration begings on l1.1
 * l1 (x) _ l1.1 (x) -> l.1.1
 *       |_ l1.2
 */
START_TEST(test_iterate_ancestors_big_tree) {
    h_tree node_l1 = NULL, node_l11 = NULL, node_l12 = NULL, node_l111 = NULL;
    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    node_l1 = tree; // For readability
    tree = h_tree_insert(tree, strdup("l1.1"), node_l1, (data_cmp_fn)strcmp);
    node_l11 = h_tree_search(tree, "l1.1", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.2"), node_l1, (data_cmp_fn)strcmp);
    node_l12 = h_tree_search(tree, "l1.2", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1.1"), node_l11, (data_cmp_fn)strcmp);
    node_l111 = h_tree_search(tree, "l1.1.1", (data_cmp_fn)strcmp);

    h_tree_iterate_h_ancestors(node_l11, (data_modify_fn)mark_node_visited);

    // Nodes that should have been modified
    fail_unless(strcmp((char *)h_tree_get_data(node_l1), "X1") == 0);
    fail_unless(strcmp((char *)h_tree_get_data(node_l11), "X1.1") == 0);
    // Nodes that should have NOT been modified
    fail_unless(strcmp((char *)h_tree_get_data(node_l12), "l1.2") == 0);
    fail_unless(strcmp((char *)h_tree_get_data(node_l111), "l1.1.1") == 0);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_flatten_h_children_null) {
    void **elem_array = NULL;
    h_tree_flatten_h_children(NULL, elem_array);
    fail_unless(errno >= 0);
    fail_unless(elem_array == NULL);
}
END_TEST

START_TEST(test_flatten_h_children_small_tree) {
    void **elem_array = calloc(1, sizeof(void *));
    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    fail_unless(tree != NULL);
    h_tree_flatten_h_children(tree, elem_array);
    fail_unless(errno >= 0);
    fail_unless(elem_array[0] == NULL);
    free(elem_array);
    h_tree_destroy(tree, free);
}
END_TEST

START_TEST(test_flatten_h_children_small_tree2) {
    h_tree node_l2 = NULL;
    void **elem_array = calloc(2, sizeof(void *));
    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1"), tree, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l2"), NULL, (data_cmp_fn)strcmp);
    node_l2 = h_tree_search(tree, "l2", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l2.1"), node_l2, (data_cmp_fn)strcmp);

    h_tree_flatten_h_children(node_l2, elem_array);

    printf("\n");
    fail_unless(errno >= 0);
    fail_unless(strcmp(elem_array[0], "l2.1") == 0);
    fail_unless(elem_array[1] == NULL);
    free(elem_array);
    h_tree_destroy(tree, free);
}
END_TEST

/* Tree used for test. We ask for children of l1.
 * l1  _ l1.1 (x) -> l.1.1
 *    |_ l1.2 (x)
 */
START_TEST(test_flatten_h_children_big_tree) {
    h_tree node_l1 = NULL, node_l11 = NULL;
    void **elem_array = calloc(3, sizeof(void *));

    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    node_l1 = tree; // For readability
    tree = h_tree_insert(tree, strdup("l1.1"), node_l1, (data_cmp_fn)strcmp);
    node_l11 = h_tree_search(tree, "l1.1", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.2"), node_l1, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1.1"), node_l11, (data_cmp_fn)strcmp);

    h_tree_flatten_h_children(node_l1, elem_array);
    fail_unless(errno >= 0);
    // Nodes are inserted at the start of the parent->h_children list.
    fail_unless(strcmp(elem_array[0], "l1.2") == 0);
    fail_unless(strcmp(elem_array[1], "l1.1") == 0);
    fail_unless(elem_array[2] == NULL);

    free(elem_array);
    h_tree_destroy(tree, free);
}
END_TEST

/* Tree used for test. We ask for children of l1.
 * l1  _ l1.1 (x) -> l.1.1
 *    |_ l1.2 (deleted)
 *    |_ l1.3 (x)
 */
START_TEST(test_flatten_h_children_delete_middle) {
    h_tree node_l1 = NULL, node_l11 = NULL;
    void **elem_array = calloc(3, sizeof(void *));

    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    node_l1 = tree; // For readability
    tree = h_tree_insert(tree, strdup("l1.1"), node_l1, (data_cmp_fn)strcmp);
    node_l11 = h_tree_search(tree, "l1.1", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.2"), node_l1, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.3"), node_l1, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1.1"), node_l11, (data_cmp_fn)strcmp);
    // Delete the middle children of l1
    tree = h_tree_delete(tree, "l1.2", (data_cmp_fn)strcmp, free);

    h_tree_flatten_h_children(node_l1, elem_array);
    fail_unless(errno >= 0);
    // Nodes are inserted at the start of the parent->h_children list.
    fail_unless(strcmp(elem_array[0], "l1.3") == 0);
    fail_unless(strcmp(elem_array[1], "l1.1") == 0);
    fail_unless(elem_array[2] == NULL);

    free(elem_array);
    h_tree_destroy(tree, free);
}
END_TEST

/* Tree used for test. We remove l1.1
 * l1  _ l1.1 (x)
 *    |_ l1.2 (x) -> l1.2.1
 */
START_TEST(test_flatten_h_children_delete_last) {
    h_tree node_l1 = NULL, node_l12 = NULL;
    void **elem_array = calloc(3, sizeof(void *));

    tree = h_tree_insert(NULL, strdup("l1"), NULL, (data_cmp_fn)strcmp);
    node_l1 = tree; // For readability
    tree = h_tree_insert(tree, strdup("l1.2"), node_l1, (data_cmp_fn)strcmp);
    node_l12 = h_tree_search(tree, "l1.2", (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.2.1"), node_l12, (data_cmp_fn)strcmp);
    tree = h_tree_insert(tree, strdup("l1.1"), node_l1, (data_cmp_fn)strcmp);
    tree = h_tree_delete(tree, strdup("l1.1"), (data_cmp_fn)strcmp, free);

    h_tree_flatten_h_children(node_l1, elem_array);
    fail_unless(errno >= 0);
    // Nodes are inserted at the start of the parent->h_children list.
    fail_unless(strcmp(elem_array[0], "l1.2") == 0);
    fail_unless(elem_array[1] == NULL);

    free(elem_array);
    h_tree_destroy(tree, free);
}
END_TEST

Suite *hierarchy_tree_suite(void) {
    Suite *test_suit = suite_create("hierarchy_tree");
    TCase *tcase_functionality = tcase_create("Hierarchy Tree functions");
    tcase_add_test(tcase_functionality, test_insert_with_parent);
    tcase_add_test(tcase_functionality, test_iterate_ancestors_null);
    tcase_add_test(tcase_functionality, test_iterate_ancestors_small_tree);
    tcase_add_test(tcase_functionality, test_iterate_ancestors_small_tree2);
    tcase_add_test(tcase_functionality, test_iterate_ancestors_big_tree);
    tcase_add_test(tcase_functionality, test_flatten_h_children_null);
    tcase_add_test(tcase_functionality, test_flatten_h_children_small_tree);
    tcase_add_test(tcase_functionality, test_flatten_h_children_small_tree2);
    tcase_add_test(tcase_functionality, test_flatten_h_children_big_tree);
    tcase_add_test(tcase_functionality, test_flatten_h_children_delete_middle);
    tcase_add_test(tcase_functionality, test_flatten_h_children_delete_last);
    suite_add_tcase(test_suit, tcase_functionality);

    return test_suit;
}

int main() {
    SRunner *runner = srunner_create(NULL);

    srunner_add_suite(runner, binary_search_tree_suite());
    srunner_add_suite(runner, hierarchy_tree_suite());

    srunner_set_log(runner, "test.log");
    srunner_run_all(runner, CK_NORMAL);
    srunner_free(runner);
    return 0;
}
