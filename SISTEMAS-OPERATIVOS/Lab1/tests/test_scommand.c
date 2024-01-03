#include <check.h>
#include "test_scommand.h"

#include <signal.h>
#include <assert.h>
#include <string.h> /* para strcmp */
#include <stdlib.h> /* para calloc */
#include <stdio.h> /* para sprintf */

#include "command.h"

#define MAX_LENGTH 257 /* no hay nada como un primo para molestar */

static scommand scmd = NULL; /* para armar scommand temporales */

/* Testeo precondiciones. 
 * Se espera que todos estos tests fallen con un assert(), son todas 
 * llamadas inválidas 
 */
START_TEST (test_destroy_null)
{
    scommand_destroy (NULL);
}
END_TEST


START_TEST (test_push_back_null)
{
    scommand_push_back (NULL, strdup ("123"));
}
END_TEST

START_TEST (test_push_back_argument_null)
{
    scmd = scommand_new ();
    scommand_push_back (scmd, NULL);
    scommand_destroy (scmd); scmd = NULL;
}
END_TEST

START_TEST (test_pop_front_null)
{
    scommand_pop_front (NULL);
}
END_TEST

START_TEST (test_pop_front_empty)
{
    scmd = scommand_new();
    scommand_pop_front (scmd);
    scommand_destroy(scmd); scmd = NULL;
}
END_TEST

START_TEST (test_set_redir_in_null)
{
    scommand_set_redir_in (NULL, strdup("123"));
}
END_TEST

START_TEST (test_set_redir_out_null)
{
    scommand_set_redir_out (NULL, strdup("123"));
}
END_TEST

START_TEST (test_is_empty_null)
{
    scommand_is_empty (NULL);
}
END_TEST

START_TEST (test_length_null)
{
    scommand_length (NULL);
}
END_TEST

START_TEST (test_front_null)
{
    scommand_front (NULL);
}
END_TEST

START_TEST (test_front_empty)
{
    scmd = scommand_new();
    scommand_front (scmd);
    scommand_destroy(scmd); scmd = NULL;
}
END_TEST

START_TEST (test_get_redir_in_null)
{
    scommand_get_redir_in (NULL);
}
END_TEST

START_TEST (test_get_redir_out_null)
{
    scommand_get_redir_out (NULL);
}
END_TEST

START_TEST (test_to_string_null)
{
    scommand_to_string (NULL);
}
END_TEST


/* Crear y destruir */
START_TEST (test_new_destroy)
{
    scmd = scommand_new ();
    /* Verificamos que se pueda crear y destruir un scommand sin problemas */
    scommand_destroy (scmd); scmd = NULL;
}
END_TEST

START_TEST (test_new_is_empty)
{
    scmd = scommand_new ();
    /* Un comando recién creado debe ser vacío */
    ck_assert_msg (scommand_is_empty (scmd), NULL);
    ck_assert_msg (scommand_length (scmd) == 0, NULL);
    scommand_destroy (scmd); scmd = NULL;
}
END_TEST

/* Testeo de funcionalidad */

static void setup (void) {
    scmd = scommand_new ();
}

static void teardown (void) {
    if (scmd != NULL) {
        scommand_destroy (scmd);
        scmd = NULL;
    }
}

/* is_empty sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying)
{
    unsigned int i = 0;
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg ((i==0) == scommand_is_empty (scmd), NULL);
        scommand_push_back (scmd, strdup ("123"));
    }
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (!scommand_is_empty(scmd), NULL);
        scommand_pop_front (scmd);
    }
    ck_assert_msg (scommand_is_empty (scmd), NULL);
}
END_TEST

/* length sea acorde a lo que agregamos y quitamos */
START_TEST (test_adding_emptying_length)
{
    unsigned int i = 0;
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (i == scommand_length (scmd), NULL);
        scommand_push_back (scmd, strdup ("123"));
    }
    for (i=MAX_LENGTH; i>0; i--) {
        ck_assert_msg (i == scommand_length (scmd), NULL);
        scommand_pop_front (scmd);
    }
    ck_assert_msg (0 == scommand_length (scmd), NULL);
}
END_TEST

static char **
numbers_as_str(int max_number) {
    assert(max_number > 0);
    char **strings=NULL;
    // Calculate number of digits
    int digits=1, rest=max_number-1;
    while(rest != 0) {
        digits++;
        rest /= 10;
    }
    // Construct string array
    strings = calloc (max_number, sizeof(char*));
    for (int i=0; i < max_number; i++) {
        char *asc_num = malloc(sizeof(char) * (digits + 1));
        sprintf(asc_num," %d", i);
        strings[i] = asc_num;
    }
    return strings;
}
/* Meter por atrás y sacar por adelante, da la misma secuencia. */
START_TEST (test_fifo)
{
    unsigned int i = 0;
    char **strings = numbers_as_str(MAX_LENGTH);
    /* strings = ["0","1", ..., "256"] */
    for (i=0; i<MAX_LENGTH; i++) {
        /* Copia antes de push: scommand se apropia */
        scommand_push_back (scmd, strdup(strings[i]));
    }
    for (i=0; i<MAX_LENGTH; i++) {
        /* mismo string */
        ck_assert_msg (strcmp (scommand_front (scmd),strings[i]) == 0, NULL);
        free(strings[i]);
        scommand_pop_front (scmd);
    }
    free (strings);
}
END_TEST

/* hacer muchísimas veces front es lo mismo */
START_TEST (test_front_idempotent)
{
    unsigned int i = 0;
    scommand_push_back (scmd, strdup ("123"));
    for (i=0; i<MAX_LENGTH; i++) {
        ck_assert_msg (strcmp (scommand_front (scmd), "123") == 0, NULL);
    }
}
END_TEST

/* Si hay solo uno, entonces front=back */
START_TEST (test_front_is_back)
{
    scommand_push_back (scmd, strdup ("123"));
    ck_assert_msg (strcmp (scommand_front (scmd), "123") == 0, NULL);
}
END_TEST

/* Si hay dos distintos entonces front!=back */
START_TEST (test_front_is_not_back)
{
    scommand_push_back(scmd, strdup ("123"));
    scommand_push_back(scmd, strdup ("456"));
    ck_assert_msg (strcmp (scommand_front (scmd), "456") != 0, NULL);
}
END_TEST

/* Que la tupla de redirectores sea un par independiente */
START_TEST (test_redir)
{
    scommand_set_redir_in (scmd, strdup ("123"));
    scommand_set_redir_out (scmd, strdup ("456"));
    /* Los redirectores tienen que ser distintos */
    ck_assert_msg (strcmp (scommand_get_redir_in (scmd),
                         scommand_get_redir_out (scmd)) != 0, NULL);
    /* ahora si ambos idem */
    scommand_set_redir_out (scmd, strdup ("123"));
    ck_assert_msg (strcmp (scommand_get_redir_in (scmd), scommand_get_redir_out (scmd)) == 0, NULL);
}
END_TEST

START_TEST (test_independent_redirs)
{
    /* Cuando la gente copia y pega entre las funciones de redir_in y 
     * redir_out, tiende a olvidarse de hacer s/in/out/. Probamos algunos
     * casos simples
     */

    /* Primero: sólo entrada */
    scommand_set_redir_in (scmd, strdup ("123"));
    ck_assert_msg (strcmp (scommand_get_redir_in (scmd), "123") == 0, NULL);
    ck_assert_msg (scommand_get_redir_out (scmd) == NULL, NULL);

    /* Segundo: Volvemos ambos a NULL */
    scommand_set_redir_in (scmd, NULL);
    ck_assert_msg (scommand_get_redir_in (scmd) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (scmd) == NULL, NULL);

    /* Tercero: sólo salida */
    scommand_set_redir_out (scmd, strdup ("456"));
    ck_assert_msg (scommand_get_redir_in (scmd) == NULL, NULL);
    ck_assert_msg (strcmp (scommand_get_redir_out (scmd), "456") == 0, NULL);

    /* Cuarto: ambos */
    scommand_set_redir_in (scmd, strdup ("123"));
    ck_assert_msg (strcmp (scommand_get_redir_in (scmd), "123") == 0, NULL);
    ck_assert_msg (strcmp (scommand_get_redir_out (scmd), "456") == 0, NULL);
}
END_TEST


/* Comando nuevo, string vacío */
START_TEST (test_to_string_empty)
{
    char *str = NULL;
    str = scommand_to_string (scmd);
    ck_assert_msg (strlen (str) == 0, NULL);
    free (str);
}
END_TEST

/* Algo más fuerte. Poner muchos argumentos, mirar el orden 
 * Poner redirectores, mirar el orden
 */
START_TEST (test_to_string)
{
    char *str = NULL;
    char **strings = NULL;
    int i = 0;
    char *last_ocurr=NULL;
    strings = numbers_as_str(MAX_LENGTH);
    /* strings = ["0","1", ..., "127"] */

    assert (MAX_LENGTH>2);
    /* comando "0 1 2 .... N-3 < N-2 > N-1" tiene que tener todos los números y los dos piquitos */
    for (i=0; i < MAX_LENGTH - 2; i++) {
        scommand_push_back (scmd, strings[i]);
    }
    scommand_set_redir_in(scmd, strings[MAX_LENGTH - 2]);
    scommand_set_redir_out(scmd, strings[MAX_LENGTH - 1]);
    str = scommand_to_string(scmd);
    last_ocurr = str;
    for (i=0; i < MAX_LENGTH - 2; i++) {
            ck_assert_msg(strstr(str, strings[i])>=last_ocurr, NULL);
            last_ocurr = strstr(str, strings[i]);
    }
    // Redir in
    ck_assert_msg (strstr(str, strings[MAX_LENGTH - 2]) != NULL, NULL);
    ck_assert_msg (strchr(str, '<') != NULL, NULL);
    ck_assert_msg (strstr(str,strings[MAX_LENGTH - 2]) > strchr(str, '<'), NULL);
    // Redir out
    ck_assert_msg (strstr(str, strings[MAX_LENGTH - 1]) != NULL, NULL);
    ck_assert_msg (strchr(str, '>') != NULL, NULL);
    ck_assert_msg (strstr(str, strings[MAX_LENGTH - 1]) > strchr(str, '>'), NULL);
    free (str);
    free(strings);
}
END_TEST


/* Armado de la test suite */

Suite *scommand_suite (void)
{
    Suite *s = suite_create ("scommand");
    TCase *tc_preconditions = tcase_create ("Precondition");
    TCase *tc_creation = tcase_create ("Creation");
    TCase *tc_functionality = tcase_create ("Functionality");

    /* Precondiciones */
    tcase_add_test_raise_signal (tc_preconditions, test_destroy_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_push_back_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_push_back_argument_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_pop_front_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_pop_front_empty, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_set_redir_in_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_set_redir_out_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_is_empty_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_length_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_front_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_front_empty, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_get_redir_in_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_get_redir_out_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_to_string_null, SIGABRT);
    suite_add_tcase (s, tc_preconditions);

    /* Creation */
    tcase_add_test (tc_creation, test_new_destroy);
    tcase_add_test (tc_creation, test_new_is_empty);
    suite_add_tcase (s, tc_creation);

    /* Funcionalidad */
    tcase_add_checked_fixture (tc_functionality, setup, teardown);
    tcase_add_test (tc_functionality, test_adding_emptying);
    tcase_add_test (tc_functionality, test_adding_emptying_length);
    tcase_add_test (tc_functionality, test_fifo);
    tcase_add_test (tc_functionality, test_front_idempotent);
    tcase_add_test (tc_functionality, test_front_is_back);
    tcase_add_test (tc_functionality, test_front_is_not_back);
    tcase_add_test (tc_functionality, test_redir);
    tcase_add_test (tc_functionality, test_independent_redirs);
    tcase_add_test (tc_functionality, test_to_string_empty);
    tcase_add_test (tc_functionality, test_to_string);
    suite_add_tcase (s, tc_functionality);

    return s;
}

/* Para testing de memoria */
void scommand_memory_test (void) {
    /* Las siguientes operaciones deberían poder hacer sin leaks ni doble 
     * frees.
     */

    /* Crear y destruir un scommand vacío */
    scmd = scommand_new ();
    scommand_destroy (scmd);

    /* Crear un scommand, llenarlo de argumentos, liberarlo
     * sin liberar los argumentos desde afuera
     */
    scmd = scommand_new ();
    scommand_push_back (scmd, strdup ("un-argumento"));
    scommand_push_back (scmd, strdup ("otro-argumento"));
    scommand_destroy(scmd);

    /* Crear un scommand, setearle redirectores, liberarlo
     * sin liberar los redirectores desde afuera
     */
    scmd = scommand_new ();
    scommand_set_redir_in (scmd, strdup ("entrada"));
    scommand_set_redir_out (scmd, strdup ("salida"));
    scommand_destroy(scmd);

    /* Meter, sacar, meter argumentos alternadamente */
    scmd = scommand_new ();
    scommand_push_back (scmd, strdup ("uno"));
    scommand_push_back (scmd, strdup ("dos"));
    scommand_push_back (scmd, strdup ("cinco"));
    scommand_pop_front (scmd);
    scommand_push_back (scmd, strdup ("perdon, tres"));
    scommand_push_back (scmd, strdup ("cuatro"));
    scommand_destroy(scmd);

    /* Modificar redictores ya seteados */
    scmd = scommand_new ();
    scommand_set_redir_out (scmd, strdup ("entrada"));
    scommand_set_redir_out (scmd, strdup ("quise decir salida"));
    scommand_destroy(scmd);

    /* Usar todos los accesores */
    scmd = scommand_new ();
    scommand_push_back (scmd, strdup ("comando"));
    scommand_push_back (scmd, strdup ("un-argumento"));
    scommand_push_back (scmd, strdup ("otro-argumento"));
    scommand_set_redir_out (scmd, strdup ("salida"));
    scommand_is_empty (scmd);
    scommand_length (scmd);
    scommand_front (scmd);
    scommand_get_redir_in (scmd);
    scommand_get_redir_out (scmd);
    scommand_destroy (scmd);
    
    /* Se puede modificar el TAD luego de acceder a front, aún cuando eso
     * invalida el resultado anterior de front. Sin leaks, al igual que todos
     * estos casos.
     */
    scmd = scommand_new ();
    scommand_push_back (scmd, strdup ("comando"));
    scommand_front (scmd);
    scommand_pop_front (scmd);
    /* En este punto, s puede ser no válido. */
    scommand_destroy (scmd);
     
    /* Al string que me devuelve to_string lo puedo liberar. Y por dentro, 
     * no hay leaks, al igual que todos estos casos.
     */
    scmd = scommand_new ();
    scommand_push_back (scmd, strdup ("comando"));
    scommand_push_back (scmd, strdup ("argument"));
    scommand_set_redir_out (scmd, strdup ("salida"));
    free (scommand_to_string (scmd));
    scommand_destroy (scmd);
}

