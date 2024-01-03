#include <stdlib.h>
#include <stdbool.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) {
    scommand new_cmd = scommand_new();
    arg_kind_t arg_type;
    char* arg;

    while (!parser_at_eof(p)) {
        
        arg = parser_next_argument(p, &arg_type); // if next argument is | => arg = NULL and | is not consumed

        if (arg != NULL) {
            if (arg_type == ARG_NORMAL) {
               scommand_push_back(new_cmd, arg);
            } else if (arg_type == ARG_INPUT) {
                scommand_set_redir_in(new_cmd, arg);
            } else if (arg_type == ARG_OUTPUT) {
                scommand_set_redir_out(new_cmd, arg);
            }
        } else if (arg_type == ARG_INPUT || arg_type == ARG_OUTPUT) {
            return NULL;
        } else {
            break;
        }

    }

    return new_cmd;
}

pipeline parse_pipeline(Parser p) {
    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool another_pipe=true;

    while (another_pipe) {
        cmd = parse_scommand(p);                                // parse p until first pipe "|" 
        if (!cmd) {
            printf("Error: no input or output file.\n");
            pipeline_destroy(result);
            return NULL;
        }
        parser_skip_blanks(p);                                  // skip blanks of parsed scom
        pipeline_push_back(result, cmd);                        // add scom to pipeline
        parser_op_pipe(p, &another_pipe);                       // checks new pipe (if pipe is found, it's consumed)
    }

    bool is_background;
    parser_op_background(p, &is_background);
    pipeline_set_wait(result, !is_background);
    


    bool gb_left;
    parser_garbage(p, &gb_left);

    if (pipeline_length(result) == 1 && scommand_length(pipeline_front(result)) == 0) {
        pipeline_destroy(result);
        return NULL;
    }

    if (gb_left) {
        printf("Error: invalid command.\n");
        pipeline_destroy(result);
        return NULL;
    }

    return result;
}

