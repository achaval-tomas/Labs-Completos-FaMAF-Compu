#include <check.h>
#include "test_pipeline.h"

#include <signal.h>
#include <assert.h>
#include <string.h> /* para strcmp */
#include <stdlib.h> /* para calloc */
#include <stdio.h> /* para sprintf */
#include "command.h"

#define MAX_LENGTH 257 /* no hay nada como un primo para molestar */

static pipeline pipe = NULL; /* para armar pipelines temporales */

/* Testeo precondiciones */
START_TEST (test_destroy_null)
{
    pipeline_destroy (NULL);
}
END_TEST


START_TEST (test_push_back_null)
{
    scommand scmd = scommand_new ();
    pipeline_push_back (NULL, scmd);
    scommand_destroy (scmd);
}
END_TEST

START_TEST (test_push_back_scmd_null)
{
    pipe = pipeline_new ();
    pipeline_push_back (pipe, NULL);
    pipeline_destroy (pipe); pipe = NULL;
}
END_TEST

START_TEST (test_pop_front_null)
{
    pipeline_pop_front (NULL);
}
END_TEST

START_TEST (test_pop_front_empty)
{
    pipe = pipeline_new();
    pipeline_pop_front (pipe);
    pipeline_destroy(pipe); pipe = NULL;
}
END_TEST

START_TEST (test_set_wait_null)
{
    pipeline_set_wait (NULL, false);
}
END_TEST

START_TEST (test_is_empty_null)
{
    pipeline_is_empty (NULL);
}
END_TEST

START_TEST (test_length_null)
{
    pipeline_length(NULL);
}
END_TEST

START_TEST (test_front_null)
{
    pipeline_front (NULL);
}
END_TEST

START_TEST (test_front_empty)
{
    pipe = pipeline_new();
    pipeline_front (pipe);
    pipeline_destroy(pipe); pipe = NULL;
}
END_TEST

START_TEST (test_get_wait_null)
{
    pipeline_get_wait (NULL);
}
END_TEST

START_TEST (test_to_string_null)
{
    pipeline_to_string (NULL);
}
END_TEST


/* Crear y destruir */
START_TEST (test_new_destroy)
{
    pipe = pipeline_new ();
    pipeline_destroy (pipe); pipe = NULL;
}
END_TEST

/* Testeo de funcionalidad */

static void setup (void) {
    pipe = pipeline_new ();
}

static void teardown (void) {
    if (pipe != NULL) {
        pipeline_destroy (pipe);
        pipe = NULL;
    }
}

/* is_empty sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying)
{
    unsigned int i = 0;

    ck_assert_msg (pipeline_is_empty (pipe), NULL);
    for (i=0; i<MAX_LENGTH; i++) {
        pipeline_push_back (pipe, scommand_new ());
        ck_assert_msg (!pipeline_is_empty (pipe), NULL);
    }
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (!pipeline_is_empty (pipe), NULL);
        pipeline_pop_front (pipe);
    }
    ck_assert_msg (pipeline_is_empty (pipe), NULL);
}
END_TEST

/* length sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying_length)
{
    unsigned int i = 0;

    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (i == pipeline_length (pipe), NULL);
        pipeline_push_back (pipe, scommand_new ());
    }
    for (i=MAX_LENGTH; i>0; i--) {
        ck_assert_msg (i == pipeline_length (pipe), NULL);
        pipeline_pop_front (pipe);
    }
    ck_assert_msg (0 == pipeline_length (pipe), NULL);
}
END_TEST

/* Meter por atrás y sacar por adelante, da la misma secuencia.
 * Reviso además que sea la misma memoria.
 */
START_TEST (test_fifo)
{
    unsigned int i = 0;
    scommand *scmds = NULL;
    scmds = calloc (MAX_LENGTH, sizeof(scommand));
    for (i=0; i<MAX_LENGTH; i++) {
        scmds[i] = scommand_new ();
    }
    for (i=0; i<MAX_LENGTH; i++) {
        pipeline_push_back (pipe, scmds[i]);
    }
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (pipeline_front (pipe) == scmds[i], NULL);
        pipeline_pop_front (pipe);
    }
}
END_TEST

/* hacer muchísimas veces front es lo mismo */
START_TEST (test_front_idempotent)
{
    unsigned int i = 0;
    scommand scmd = scommand_new ();
    pipeline_push_back (pipe, scmd);
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (pipeline_front (pipe) == scmd, NULL);
    }
}
END_TEST

/* Si hay solo uno, entonces front=back */
START_TEST (test_front_is_back)
{
    scommand scmd = scommand_new ();
    pipeline_push_back (pipe, scmd);
    ck_assert_msg (pipeline_front (pipe)==scmd, NULL);
}
END_TEST

/* Si hay dos distintos entonces front!=back */
START_TEST (test_front_is_not_back)
{
    scommand scmd0 = scommand_new ();
    scommand scmd1 = scommand_new ();
    pipeline_push_back (pipe, scmd0);
    pipeline_push_back (pipe, scmd1);
    ck_assert_msg (pipeline_front (pipe) != scmd1, NULL);
}
END_TEST

START_TEST (test_wait)
{
    pipeline_set_wait (pipe, true);
    ck_assert_msg (pipeline_get_wait (pipe), NULL);
    pipeline_set_wait (pipe, false);
    ck_assert_msg (!pipeline_get_wait (pipe), NULL);
}
END_TEST

/* Comando nuevo, string vacío */
START_TEST (test_to_string_empty)
{
    char *str = NULL;
    str = pipeline_to_string (pipe);
    ck_assert_msg (strlen (str) == 0, NULL);
    free (str); str = NULL;
}
END_TEST

/* Armamos un pipeline de n y contamos que haya n-1 '|'.
 * Que no espere y buscamos el '&' al final.
 */
START_TEST (test_to_string)
{
    char *str = NULL;
    /* MAX_LENGTH veces el mismo comando simple */
    for (int i=0; i<MAX_LENGTH; i++) {
        scommand cmd=scommand_new();
        scommand_push_back(cmd, strdup ("gtk-fuse"));
        pipeline_push_back (pipe, cmd);
    }
    pipeline_set_wait (pipe, false);
    str = pipeline_to_string (pipe);
    /* cuenta cuantos pipes hay en n */
    int i=0;
    int n=0;
    while (str[i]) {
        n += (str[i] == '|');
        i++;
    }
    ck_assert_msg (n==MAX_LENGTH-1, NULL);
    ck_assert_msg (n < 2 || strchr(strrchr(str, '|'), '&') != NULL, NULL);
    
    free (str);
}
END_TEST

/* Armado de la test suite */

Suite *pipeline_suite (void)
{
    Suite *s = suite_create ("pipeline");
    TCase *tc_preconditions = tcase_create ("Precondition");
    TCase *tc_creation = tcase_create ("Creation");
    TCase *tc_functionality = tcase_create ("Functionality");

    /* Precondiciones */
    tcase_add_test_raise_signal (tc_preconditions, test_destroy_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_push_back_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_push_back_scmd_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_pop_front_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_pop_front_empty, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_set_wait_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_is_empty_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_length_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_front_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_front_empty, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_get_wait_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_to_string_null, SIGABRT);
    suite_add_tcase (s, tc_preconditions);

    /* Creation */
    tcase_add_test (tc_creation, test_new_destroy);
    suite_add_tcase (s, tc_creation);

    /* Funcionalidad */
    tcase_add_checked_fixture (tc_functionality, setup, teardown);
    tcase_add_test (tc_functionality, test_adding_emptying);
    tcase_add_test (tc_functionality, test_adding_emptying_length);
    tcase_add_test (tc_functionality, test_fifo);
    tcase_add_test (tc_functionality, test_front_idempotent);
    tcase_add_test (tc_functionality, test_front_is_back);
    tcase_add_test (tc_functionality, test_front_is_not_back);
    tcase_add_test (tc_functionality, test_wait);
    tcase_add_test (tc_functionality, test_to_string_empty);
    tcase_add_test (tc_functionality, test_to_string);
    suite_add_tcase (s, tc_functionality);

    return s;
}

/* Para testing de memoria */
void pipeline_memory_test (void) {
    char* s = NULL;
    /* Las siguientes operaciones deberían poder hacer sin leaks ni doble 
     * frees.
     */
    /* Crear y destruir un pipe vacío */
    pipe = pipeline_new ();
    pipeline_destroy (pipe);
    /* Crear y destruir un pipe con varios comandos */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_destroy (pipe);
    /* Lo mismo, toqueteando el flag de wait */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_set_wait (pipe, false);
    pipeline_push_back (pipe, scommand_new ());
    pipeline_set_wait (pipe, true);
    pipeline_destroy (pipe);
    /* Insertar, sacar, insertar de vuelta */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_pop_front (pipe);
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_destroy (pipe);
    /* Usar todos los accesores */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_is_empty (pipe);
    pipeline_length (pipe);
    pipeline_front (pipe);
    pipeline_get_wait (pipe);
    pipeline_destroy (pipe);
    /* Se puede modificar el TAD luego de acceder a front, aún cuando eso
     * invalida el resultado anterior de front. Sin leaks, al igual que todos
     * estos casos.
     */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    pipeline_front (pipe);
    pipeline_pop_front (pipe);
    /* En este punto, c puede ser no válido. */
    pipeline_destroy (pipe);
    /* Al string que me devuelve to_string lo puedo liberar. Y por dentro, 
     * no hay leaks, al igual que todos estos casos.
     */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    pipeline_push_back (pipe, scommand_new ());
    s = pipeline_to_string (pipe);
    pipeline_destroy (pipe);
    free(s);
    /* Se le puede meter mano a los comandos dentro del pipe, y todo debería
     * andar bien y sin leaks ni double frees si no destruyo nada.
     */
    pipe = pipeline_new ();
    pipeline_push_back (pipe, scommand_new ());
    scommand_push_back (pipeline_front (pipe), strdup ("test"));
    pipeline_destroy (pipe);
}

