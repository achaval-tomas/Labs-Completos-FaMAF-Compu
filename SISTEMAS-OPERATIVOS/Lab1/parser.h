#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>    /* bool */
#include <stdio.h>      /* FILE */
#include "command.h"    /* pipeline */

/* Tipo opaco, implementación oculta */
typedef struct parser_s * Parser;

typedef enum {
    ARG_NORMAL, // Indicates a command name or command argument type
    ARG_INPUT,  // Indicates an input redirection
    ARG_OUTPUT  // Indicates an output redirection
} arg_kind_t; // An auxiliary type for parser_next_argument() 

Parser parser_new(FILE *input);
/*
 * Constructor de Parser.
 * El input es el archivo de donde se quieren parsear pipelines.
 * REQUIRES:
 *     input != NULL
 * ENSURES:
 *     Devuelve un Parser para el archivo
 *     o NULL en caso de haber un error de inicialización
 */


Parser parser_destroy(Parser parser);
/*
 * Destructor de Parser.
 * REQUIRES:
 *     parser != NULL
 * ENSURES:
 *     Devuelve NULL
 */


char * parser_next_argument(Parser parser, arg_kind_t *arg_type);
/*
 * Procesa el próximo argumento e indica si corresponde a un argumento normal, o
 * si es una redirección de entrada/salida. Si se encuentra un símbolo del
 * operador pipe (|) o un fin de línea (\n), el procesamiento no avanza, dejando
 * sin consumir dichos símbolos.
 *
 * ls -l -a  > salida.txt < entrada.txt | wc -l     &  \n
 *
 * cat /proc/cpuinfo | grep model
 *  
 *  Salida 1: "ls", ARG_NORMAL
 *  Salida 2: "-l", ARG_NORMAL
 *  Salida 3: "-a", ARG_NORMAL
 *  Salida 4: "salida.txt", ARG_OUTPUT
 *  Salida 5: "entrada.txt", ARG_INPUT
 *
 * aa algo.txt && bb lala.txt &
 *_
 * EJEMPLO:
 *
 * arg_kind_t type;
 * char *arg;
 * arg = parser_next_argument(parser, &type);
 * if (type == ARG_NORMAL) {
 * } 
 *
 * - En `type` se guarda el tipo de argumento:
 *   + ARG_NORMAL: Era el nombre de un comando o uno de sus argumentos
 *   + ARG_INPUT: Era una redirección de entrada (algo como "< nombre_archivo")
 *   + ARG_OUTPUT: Era una redirección de salida (algo como "> nombre_archivo")
 *
 * - En `arg` se guarda la cadena procesada. En caso de que el tipo del
 *   argumento se corresponda a ARG_INPUT o ARG_OUTPUT solo se guarda
 *   "nombre_archivo" sin los símbolos "<", ">".
 *
 * El valor devuelto por la función es un puntero a memoria dinámica que queda
 * a cargo del llamador
 *
 * REQUIRES:
 *     ! parser_at_eof (parser)
 * ENSURES:
 *
 */


void parser_op_background(Parser parser, bool *was_op_background);
/*
 * Intenta leer un operador de background "&" e indica si se encontró dicho
 * operador. En caso de encontrar un "&", el operador se consume en caso
 * contrario no se consume ningún símbolo de la entrada.
 * 
 * EJEMPLO:
 *
 * bool is_background;
 * parser_op_background(parser, &is_background);
 *
 * - En `is_background` se indica si se econtró el operador
 *
 * REQUIRES:
 *     ! parser_at_eof (parser)
 * ENSURES:
 *
 */


void parser_skip_blanks(Parser parser);
/*
 * Consume todos los caracteres en blanco a continuación que hay en la entrada.
 * Detiene el procesamiento ante cualquier símbolo que no es un espacio (" ") o
 * un tabulador ("\t").
 * 
 * EJEMPLO:
 *
 * parser_skip_blanks(parser);
 *
 * REQUIRES:
 *     ! parser_at_eof (parser)
 * ENSURES:
 *
 */

void parser_op_pipe(Parser parser, bool *was_op_pipe);
/*
 * Intenta leer un operador de pipe "|" e indica si se encontró dicho
 * operador. En caso de encontrar un "|", el operador se consume en caso
 * contrario no se consume ningún símbolo de la entrada.
 * 
 * EJEMPLO:
 *
 * bool is_pipe;
 * parser_op_pipe(parser, &is_pipe);
 *
 * - En `is_pipe` se indica si se encontró el operador
 *
 * REQUIRES:
 *     ! parser_at_eof (parser)
 * ENSURES:
 *
 */

void parser_garbage(Parser parser, bool *garbage);
/*
 * Consume todos los caracteres encontrados hasta un final de linea "\n" el
 * cual también se consume. Indica si se encontraron símbolos distintos de
 * espacios en blanco en el medio del procesamiento.
 *
 *
 * NOTA: Es la única función del TAD que consume un "\n"
 * 
 *
 * EJEMPLO:
 *
 * bool garbage;
 * parser_garbage(parser, &garbage);
 *
 * - En `garbage` se indica si se econtraron caracteres distintos de espacios
 *   en blanco
 *
 * REQUIRES:
 *     ! parser_at_eof (parser) && garbage != NULL
 *
 */

char * parser_last_garbage(Parser parser);
/*
 * Devuelve una cadena con los símbolos leídos en la última llamada a
 * `parser_garbage()` en donde se haya encontrado basura. La cadena devuelta es
 * propiedad del TAD y NO DEBE SER LIBERADA.
 *
 * REQUIRES:
 *     ! parser_at_eof (parser)
 *
 */

bool parser_at_eof(Parser parser);
/*
 * Consulta si el parser llegó al final del archivo.
 * REQUIRES:
 *     parser != NULL
 */

#endif /* PARSER_H */

