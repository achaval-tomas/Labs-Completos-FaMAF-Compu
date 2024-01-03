#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "tests/syscall_mock.h"

#include "builtin.h"
#include "command.h"


static const char* BUILTIN_CMDS[] = { "cd",
                                      "help",
                                      "exit"
};
static const unsigned int BUILTIN_SIZE = 3;

static char home_dir[] = "/home/";

bool builtin_is_internal(scommand cmd) {
    assert(cmd != NULL);
    for (unsigned int i = 0; i < BUILTIN_SIZE; ++i) {
        if (!strcmp(scommand_front(cmd), BUILTIN_CMDS[i]))
            return true;
    }

    return false;
}

bool builtin_alone(pipeline p){
    assert(p != NULL);
    return (pipeline_length(p) == 1) && builtin_is_internal(pipeline_front(p));
}


void builtin_run(scommand cmd){
    assert(builtin_is_internal(cmd));
    
    if (!strcmp(scommand_front(cmd), BUILTIN_CMDS[0])) {
        // cd 
        scommand_pop_front(cmd);          // front = cd
        
        if (scommand_length(cmd) == 0){      // "cd" by itself chdirs to home/username dir.
            char* user_name = getlogin();
            char* user_dir = malloc((strlen(home_dir) + strlen(user_name) + 1) * sizeof(char));
            strcpy(user_dir, home_dir);
            strcat(user_dir, user_name);
            scommand_push_back(cmd, user_dir);
        }

        int rc = chdir(scommand_front(cmd));  // chdir = newpatherson

        
        if (rc == -1) { 
            // hubo error
            switch (errno) {
            case EACCES:
                break;
            case ENOENT:
                printf("There is no such directory\n");
                break;
            case ENOTDIR:
                printf("The specified path is not a directory\n");
                break;
            default:
                printf("Error when trying to change dir\n");
                break;
            }
        }
        
    } else if (!strcmp(scommand_front(cmd), BUILTIN_CMDS[1])) {
            // help
            printf("This Builtin manual has been created by The Handstand Fellow, The Indian Guy, Van Basten (prime) and Death \"Destroyer of Worlds\" \n"
                   "You could use these commands: 1) 'cd', 2) 'help', 3) 'exit'\n"
                   "1) 'cd <pathname>' let us change the current directory\n"
                   "2) 'help' give you a brief idea and the functionality of each command\n"
                   "3) 'exit' quit the actual bash.\n");
    } else {
            // exit
            exit(EXIT_SUCCESS);
    } 
}
