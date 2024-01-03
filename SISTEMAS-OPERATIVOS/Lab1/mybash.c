#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

static char curworkdir[1000];

static void show_prompt(void) {
    char* buf_ptr = getcwd(curworkdir, 1000);
    if (buf_ptr == NULL)
        printf("my_bash> ");
    else
        printf ("\x1b[42m\x1b[30m%s$\x1b[0m ", curworkdir);
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input = parser_new(stdin);

    while (!parser_at_eof(input)) {
        
        show_prompt();
        
        pipe = parse_pipeline(input);

        if (pipe != NULL) {
            execute_pipeline(pipe);
            pipeline_destroy(pipe);
        }

    }

    parser_destroy(input); input = NULL;
    printf("\n");
    return EXIT_SUCCESS;
}

