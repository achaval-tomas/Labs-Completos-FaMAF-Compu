#define _GNU_SOURCE
#include <check.h>
#include "test_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

#include "../parser.h"
#include "../parsing.h"

/* Algunas variables/funciones auxiliares para ser usadas por el resto de los
 * tests
 */

static Parser parser = NULL;
static FILE *input = NULL;
static pipeline output = NULL;

static void init_parser(char *content) {
    /* Inicializa `lexer' con un archivo simulado con `content' de
     * contenido. Para no crear un archivo temporal, usa extensiones de GNU
     */
    assert(parser==NULL);
    assert(input==NULL);

    input = fmemopen(content, strlen(content), "r");
    parser = parser_new(input);
}

static void setup (void) {
}

static void teardown (void) {
    if (parser != NULL) {
        parser_destroy(parser); parser = NULL;
    }
    if (input != NULL) {
        fclose (input); input = NULL;
    }
    if (output != NULL) {
        pipeline_destroy (output); output = NULL;
    }
}

static void check_argument (scommand cmd, const char *first) {
    char* arg = NULL;

    assert (cmd != NULL);
    assert (first != NULL);
    /* Comprueba que el primer elemento en `cmd' sea `first', y lo saca */
    ck_assert_msg (! scommand_is_empty (cmd), NULL);
    
    arg = scommand_front (cmd);
    ck_assert_msg (strcmp (arg, first) == 0, NULL);
    scommand_pop_front(cmd);
}

/* Testeo precondiciones. 
 * Se espera que todos estos tests fallen con un assert(), son todas 
 * llamadas inválidas 
 */
/*
START_TEST (test_parse_null)
{
    output = parse_pipeline (NULL);
}
END_TEST
*/

START_TEST (test_parse_closed)
{
    init_parser("x");
    /* forzamos estado de is_off en el lexer */
    arg_kind_t type;
    parser_next_argument(parser, &type);
    assert (parser_at_eof(parser));

    /* Y tratamos de usar el parser (debería fallar) */
    output = parse_pipeline(parser);
}
END_TEST

/* Algunas propiedades generales del parser. Sobre todo de cuanto 
 * input consume 
 */
START_TEST (test_consumes_until_newline)
{
    char* after = NULL;

    init_parser("ls\n--------\n\n\n");
    output = parse_pipeline(parser);

    /* Consumió hasta el \n equivocado? */
    ck_assert_msg (!parser_at_eof(parser), NULL);
    arg_kind_t type=ARG_NORMAL;
    after = parser_next_argument(parser, &type);
    /* Debería leer 8 veces "-", o sea que fue el primer \n: */
    ck_assert_msg (strlen(after) == 8, NULL);
    free(after);
}
END_TEST

START_TEST (test_consumes_until_eof)
{
    init_parser("ls");
    output = parse_pipeline(parser);

    ck_assert_msg (parser_at_eof(parser), NULL); /* No consumió todo! debería */
    ck_assert_msg (feof (input), NULL);
}
END_TEST

/* Entradas válidas. Las básicas, como para que crash se pueda usar, y
 * para sacar nota "A" 
 */

START_TEST (test_empty)
{
    //scommand s = NULL;

    init_parser("\n");
    output = parse_pipeline(parser);
    ck_assert_msg (output==NULL, NULL);

    /* Esto debería generar un pipeline de un elemento, con un comando
     * vacío adentro FIXME: Me parece que no es cierto
     */
    //ck_assert_msg (pipeline_length (output) == 1, NULL);
    //s = pipeline_front (output);
    //ck_assert_msg (scommand_length (s) == 0, NULL);
    /* Y no es en background */
    //ck_assert_msg (pipeline_get_wait (output), NULL);
    /*
     * Según como hagan el parser, una alternativa es hacer que esto genere un
     * pipeline de 0 elementos. Si lo prefieren lo pueden cambiar, pero tienen
     * que cambiar este test para que se corresponda con eso.
     */
}
END_TEST

START_TEST (test_command)
{
    scommand s = NULL;

    init_parser("comando\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de un elemento, con un comando
     * de un elemento adentro
     */
    ck_assert_msg (pipeline_length(output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 1, NULL);
    check_argument (s, "comando");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_command_with_args)
{
    scommand s = NULL;

    init_parser("comando arg1 arg2\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de un elemento, con un comando
     * de 3 elementos adentro
     */
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 3, NULL);
    check_argument(s, "comando");
    check_argument(s, "arg1");
    check_argument(s, "arg2");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in(s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out(s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait(output), NULL);
}
END_TEST

START_TEST (test_command_background)
{
    scommand s = NULL;

    init_parser("comando arg1 &\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline con el flag de no esperar */
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (! pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_command_redir_in)
{
    scommand s = NULL;

    init_parser("comando arg1 < entrada\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de un elemento, con un comando
     * redirigido adentro
     */
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* con redirección */
    ck_assert_msg (strcmp (scommand_get_redir_in (s), "entrada") == 0, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_command_redir_out)
{
    scommand s = NULL;

    init_parser("comando arg1 > salida\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de un elemento, con un comando
     * redirigido adentro
     */
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* con redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s)!=NULL, NULL);
    ck_assert_msg (strcmp (scommand_get_redir_out (s), "salida") == 0, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_command_redir_both)
{
    scommand s = NULL;

    init_parser("comando arg1 < entrada > salida\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de un elemento, con un comando
     * redirigido adentro
     */
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* con redirección */
    ck_assert_msg (scommand_get_redir_in(s)!=NULL, NULL);
    ck_assert_msg (strcmp (scommand_get_redir_in (s), "entrada") == 0, NULL);
    ck_assert_msg (scommand_get_redir_out(s)!=NULL, NULL);
    ck_assert_msg (strcmp (scommand_get_redir_out (s), "salida") == 0, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_pipe_simple)
{
    scommand s = NULL;

    init_parser("comando | filtro\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de 2 elementos, cada uno con un comando
     * simple adentro
     */
    ck_assert_msg (pipeline_length (output) == 2, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 1, NULL);
    check_argument (s, "comando");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    pipeline_pop_front (output);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 1, NULL);
    check_argument (s, "filtro");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_pipe_with_args)
{
    scommand s = NULL;

    init_parser("comando arg1 | filtro arg2\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de 2 elementos, cada uno con un comando
     * y argumento adentro
     */
    ck_assert_msg (pipeline_length (output) == 2, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    pipeline_pop_front (output);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "filtro");
    check_argument (s, "arg2");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_pipe_background)
{
    scommand s = NULL;

    init_parser("comando arg1 | filtro arg2 &\n");
    output = parse_pipeline(parser);
    /* Esto debería generar un pipeline de 2 elementos, cada uno con un comando
     * y argumento adentro
     */
    ck_assert_msg (pipeline_length (output) == 2, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "comando");
    check_argument (s, "arg1");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    pipeline_pop_front (output);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 2, NULL);
    check_argument (s, "filtro");
    check_argument (s, "arg2");
    /* Sin redirección */
    ck_assert_msg (scommand_get_redir_in (s) == NULL, NULL);
    ck_assert_msg (scommand_get_redir_out (s) == NULL, NULL);
    /* Y no es en background */
    ck_assert_msg (! pipeline_get_wait (output), NULL);
}
END_TEST

START_TEST (test_non_alphabetic_args)
{
    scommand s = NULL;
    /* Nos interesa que el parser se banque argumentos que tengan cualquier
     * caracter que no signifique algo especial. Guiones, barras, igual,
     * números son ejemplos de cosas que suelen ir en argumentos.
     */
    init_parser("comando --size=11 /etc/passwd\n");
    output = parse_pipeline(parser);
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    ck_assert_msg (scommand_length (s) == 3, NULL);
    check_argument (s, "comando");
    check_argument (s, "--size=11");
    check_argument (s, "/etc/passwd");
}
END_TEST

START_TEST (test_many_args)
{
    scommand s = NULL;
    /* Nos interesa que el parser se banque cualquier cantidad de argumentos */
#define ARGLIST_10 " 1 2 3 4 5 6 7 8 9 10"
#define ARGLIST_100 ARGLIST_10 ARGLIST_10 ARGLIST_10 ARGLIST_10 ARGLIST_10 \
                    ARGLIST_10 ARGLIST_10 ARGLIST_10 ARGLIST_10 ARGLIST_10

    init_parser("comando" ARGLIST_100 ARGLIST_100 "\n");
    output = parse_pipeline(parser);
    ck_assert_msg (pipeline_length (output) == 1, NULL);
    s = pipeline_front (output);
    /* 1 comando + 200 argumentos = 201 elementos */
    ck_assert_msg (scommand_length (s) == 201, NULL);
    check_argument (s, "comando");
    /* Supongamos que los argumentos están bien, basado en tests anteriores. */
}
END_TEST

/* Armado de la test suite */

Suite *parser_suite (void)
{
    Suite *s = suite_create ("parser");
    TCase *tc_preconditions = tcase_create ("Precondition");
    TCase *tc_general = tcase_create ("General");
    TCase *tc_valid = tcase_create ("Valid");
    TCase *tc_invalid = tcase_create ("Invalid");
    TCase *tc_valid2 = tcase_create ("Valid(hard)");
    TCase *tc_invalid2 = tcase_create ("Invalid(hard)");

    /* Precondiciones */
    tcase_add_checked_fixture (tc_preconditions, setup, teardown);
    //tcase_add_test_raise_signal (tc_preconditions, test_parse_null, SIGABRT);
    tcase_add_test_raise_signal (tc_preconditions, test_parse_closed, SIGABRT);
    suite_add_tcase (s, tc_preconditions);

    /* Propiedades generales del parser */
    tcase_add_checked_fixture (tc_general, setup, teardown);
    tcase_add_test (tc_general, test_consumes_until_newline);
    tcase_add_test (tc_general, test_consumes_until_eof);
    suite_add_tcase (s, tc_general);

    /* Entradas válidas, simples */
    tcase_add_checked_fixture (tc_valid, setup, teardown);
    tcase_add_test (tc_valid, test_empty);
    tcase_add_test (tc_valid, test_command);
    tcase_add_test (tc_valid, test_command_with_args);
    tcase_add_test (tc_valid, test_command_background);
    tcase_add_test (tc_valid, test_command_redir_in);
    tcase_add_test (tc_valid, test_command_redir_out);
    tcase_add_test (tc_valid, test_command_redir_both);
    tcase_add_test (tc_valid, test_pipe_simple);
    tcase_add_test (tc_valid, test_pipe_with_args);
    tcase_add_test (tc_valid, test_pipe_background);
    tcase_add_test (tc_valid, test_non_alphabetic_args);
    tcase_add_test (tc_valid, test_many_args);
    suite_add_tcase (s, tc_valid);

    /* Chequeos de error básicos */
    tcase_add_checked_fixture (tc_invalid, setup, teardown);
    suite_add_tcase (s, tc_invalid);

    /* Entradas válidas, complejas */
    tcase_add_checked_fixture (tc_valid2, setup, teardown);
    suite_add_tcase (s, tc_valid2);

    /* Entradas inválidas, complejas */
    tcase_add_checked_fixture (tc_invalid2, setup, teardown);
    suite_add_tcase (s, tc_invalid2);

    return s;
}

/* Para testing de memoria */
void parser_memory_test (void) {
    /* Las siguientes operaciones deberían poder hacer sin leaks ni doble 
     * frees.
     */
    init_parser ("ls");
    output = parse_pipeline(parser);
    teardown();
}

