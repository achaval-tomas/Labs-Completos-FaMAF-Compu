#include "execute.h"
#include "builtin.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glib.h>
#include <signal.h>

#include "command.h"
#include "tests/syscall_mock.h"

void execute_command(scommand exc) {
    assert(exc != NULL);

    unsigned int scommand_len = scommand_length(exc);
    char **argv = calloc(scommand_len + 1, sizeof(char *)); // array of strings that will contain the command
    unsigned int i = 0;


    while (!scommand_is_empty(exc)) {
        unsigned int length_com = strlen(scommand_front(exc)) + 1;
        argv[i] = malloc(length_com * sizeof(char));        // allocating memory for each command
        strcpy(argv[i], scommand_front(exc));               // copy into the array
        scommand_pop_front(exc);
        ++i;
    }

    char* filename_in = scommand_get_redir_in(exc);
    if (filename_in) {
        int in_redir = open(filename_in, O_RDONLY, S_IRWXU); 
        close(STDIN_FILENO);   // open input redir instead of stdin
        dup(in_redir);
        close(in_redir);
    }
    
    char* filename_out = scommand_get_redir_out(exc);
    if (filename_out) {
        int out_redir = open(filename_out, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); // open output redir instead of stdout
        close(STDOUT_FILENO);
        dup(out_redir);
        close(out_redir);
    }
    
    argv[scommand_len] = NULL;
    execvp(argv[0], argv);     // execute command with redirections all-set.
}

void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);

    if (pipeline_is_empty(apipe))
        return;

    if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe));
        return;
    }

    if (!pipeline_get_wait(apipe))  // do not leave zombies
        signal(SIGCHLD, SIG_IGN);

    unsigned int apipe_length = pipeline_length(apipe);

    int* children_pids = malloc(apipe_length * sizeof(int));
    
    int fildes[2];
    int tmp[2];

    for (unsigned int i = 0; i < apipe_length; ++i) {

        if (i != 0) {
            tmp[0] = fildes[0];
            tmp[1] = fildes[1];
        }

        if (i != apipe_length - 1) {
            pipe(fildes);
        }

       int rc = fork();

        if (rc < 0) {
            fprintf(stderr, "FORK FAILED.\n");
            return;
        } else if (rc == 0) {

            if (i != apipe_length - 1) {
                close(fildes[0]);
                close(STDOUT_FILENO);
                dup(fildes[1]);
                close(fildes[1]);
            }

            if (i != 0) {
                close(tmp[1]);
                close(STDIN_FILENO);
                dup(tmp[0]);
                close(tmp[0]);
            }

            char* command_str = scommand_to_string(pipeline_front(apipe));
            execute_command(pipeline_front(apipe));
            fprintf(stderr, "Error executing: %s\n", command_str);
            exit(EXIT_FAILURE);
        } else {
            if (i != 0) {
                close(tmp[0]);
                close(tmp[1]);
            }
            children_pids[i] = rc;
            pipeline_pop_front(apipe);
        }
    }

    if (pipeline_get_wait(apipe)) {
        for (unsigned int i = 0; i < apipe_length; ++i)
            waitpid(children_pids[i], NULL, 0);
    }
    
    free(children_pids);
}
