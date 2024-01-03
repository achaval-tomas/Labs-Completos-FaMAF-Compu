/*
 * Tests for the fat_fs_tree data sctructure
 *
 */

#include "fat_fs_tree.h"
#include <assert.h>
#include <check.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* Auxiliary functions */

static fat_tree insert_many_files(fat_tree tree, int n, char **filenames) {
    int original_size = fat_tree_size(tree);
    for (int i = 0; i < n; i++) {
        fat_file new_file = fat_file_init(NULL, false, strdup(filenames[i]));
        tree = fat_tree_insert(tree, NULL, new_file);
        fail_unless(fat_tree_size(tree) == original_size + i + 1);
    }
    return tree;
}

static void compare_tree_preorder(fat_tree tree, char **filenames) {
    char **tree_elems = (char **)fat_tree_flatten_preorder(tree);
    int n = fat_tree_size(tree);
    for (int i = 0; i < n; i++) {
        fail_unless(strcmp(filenames[i], tree_elems[i]) == 0);
    }
    free(tree_elems);
}

fat_tree tree = NULL;

START_TEST(test_init_destroy) {
    tree = fat_tree_init();
    fail_unless(fat_tree_size(tree) == 0);
    fat_tree_destroy(tree);
    tree = NULL;
}
END_TEST

START_TEST(test_destroy_null) { fat_tree_destroy(NULL); }
END_TEST

START_TEST(test_insert_null_file) {
    fat_tree new_tree = NULL;
    tree = fat_tree_init();
    new_tree = fat_tree_insert(tree, NULL, (fat_file)NULL);
    fail_unless(new_tree == NULL);
    fail_unless(errno == EINVAL);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_insert_null_tree) {
    fat_tree new_tree = NULL;
    fat_file new_file = fat_file_init(NULL, false, strdup("f"));
    new_tree = fat_tree_insert(NULL, NULL, new_file);
    fail_unless(new_tree == NULL);
    fail_unless(errno == EINVAL);
    fat_file_destroy(new_file);
}
END_TEST

START_TEST(test_insert_one) {
    fat_tree new_tree = NULL;
    fat_file new_file = fat_file_init(NULL, false, strdup("f"));
    tree = fat_tree_init();
    new_tree = fat_tree_insert(tree, NULL, new_file);
    fail_unless(fat_tree_size(new_tree) == 1);
    fail_unless(new_tree == tree);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_insert_many) {
    char *filenames[5] = {"f3", "f4", "f1", "f2", "f5"};
    tree = fat_tree_init();
    tree = insert_many_files(tree, 5, filenames);
    fail_unless(fat_tree_size(tree) == 5);
    char *expected_filenames[5] = {"f3", "f1", "f2", "f4", "f5"};
    compare_tree_preorder(tree, expected_filenames);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_search_null_tree) {
    fat_file found_node = fat_tree_search(NULL, "1");
    fail_unless(found_node == NULL);
}
END_TEST

START_TEST(test_search_null_key) {
    fat_file found_node = fat_tree_search(NULL, NULL);
    fail_unless(found_node == NULL);
}
END_TEST

START_TEST(test_search_small_tree) {
    fat_file found_node = NULL;
    fat_file new_file = fat_file_init(NULL, false, strdup("f1"));
    tree = fat_tree_init();
    tree = fat_tree_insert(tree, NULL, new_file);
    found_node = fat_tree_search(tree, "f1");
    fail_unless(found_node != NULL);
    fail_unless(fat_file_cmp_path(found_node, "f1") == 0);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_search_big_tree) {
    fat_file found_node = NULL;
    char *filenames[5] = {"f3", "f4", "f1", "f2", "f5"};
    tree = fat_tree_init();
    tree = insert_many_files(tree, 5, filenames);
    fail_unless(fat_tree_size(tree) == 5);
    found_node = fat_tree_search(tree, "f2");
    fail_unless(found_node != NULL);
    fail_unless(fat_file_cmp_path(found_node, "f2") == 0);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_search_non_existing_key) {
    fat_file found_node = NULL;
    char *filenames[5] = {"f3", "f4", "f1", "f2", "f5"};
    tree = fat_tree_init();
    tree = insert_many_files(tree, 5, filenames);
    fail_unless(fat_tree_size(tree) == 5);
    found_node = fat_tree_search(tree, "f6");
    fail_unless(found_node == NULL);
    fat_tree_destroy(tree);
}
END_TEST

START_TEST(test_increase_time_opened) {
    fat_file new_file = NULL;
    h_tree node1 = NULL, node11 = NULL, node12 = NULL, node111 = NULL;

    tree = fat_tree_init();
    new_file = fat_file_init(NULL, false, strdup("f1"));
    tree = fat_tree_insert(tree, NULL, new_file);
    node1 = fat_tree_node_search(tree, "f1");

    new_file = fat_file_init(NULL, false, strdup("f1.1"));
    tree = fat_tree_insert(tree, node1, new_file);
    node11 = fat_tree_node_search(tree, "f1.1");

    new_file = fat_file_init(NULL, false, strdup("f1.2"));
    tree = fat_tree_insert(tree, node1, new_file);
    node12 = fat_tree_node_search(tree, "f1.2");

    new_file = fat_file_init(NULL, false, strdup("f1.1.1"));
    tree = fat_tree_insert(tree, node11, new_file);
    node111 = fat_tree_node_search(tree, "f1.1.1");

    // We mock the function so instead of increasing count, it changes the name
    // to start with X.
    fat_tree_inc_num_times_opened(node11);

    // References to nodes should not have changed
    // Nodes that should have been visited
    fail_unless(fat_file_cmp(fat_tree_get_file(node11), (fat_file) "X1.1") ==
                0);
    fail_unless(fat_file_cmp(fat_tree_get_file(node1), (fat_file) "X1") == 0);
    // Nodes that should have not been visited
    fail_unless(fat_file_cmp(fat_tree_get_file(node12), (fat_file) "X1.2") !=
                0);
    fail_unless(fat_file_cmp(fat_tree_get_file(node111), (fat_file) "X1.1.1") !=
                0);

    fat_tree_destroy(tree);
}
END_TEST

/* Building the test suite */

Suite *hierarchy_tree_suite(void) {
    Suite *test_suit = suite_create("fat_fs_tree");
    TCase *tcase_functionality = tcase_create("Functionality");
    tcase_add_test(tcase_functionality, test_init_destroy);
    tcase_add_test(tcase_functionality, test_destroy_null);
    tcase_add_test(tcase_functionality, test_insert_null_file);
    tcase_add_test(tcase_functionality, test_insert_null_tree);
    tcase_add_test(tcase_functionality, test_insert_one);
    tcase_add_test(tcase_functionality, test_insert_many);
    tcase_add_test(tcase_functionality, test_search_null_tree);
    tcase_add_test(tcase_functionality, test_search_null_key);
    tcase_add_test(tcase_functionality, test_search_small_tree);
    tcase_add_test(tcase_functionality, test_search_big_tree);
    tcase_add_test(tcase_functionality, test_search_non_existing_key);
    tcase_add_test(tcase_functionality, test_increase_time_opened);
    suite_add_tcase(test_suit, tcase_functionality);

    return test_suit;
}

int main() {
    SRunner *runner = srunner_create(NULL);

    srunner_add_suite(runner, hierarchy_tree_suite());

    srunner_set_log(runner, "test.log");
    srunner_run_all(runner, CK_NORMAL);
    srunner_free(runner);
    return 0;
}
