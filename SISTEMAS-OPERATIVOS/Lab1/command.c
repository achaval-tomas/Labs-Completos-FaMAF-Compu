#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "strextra.h"

#include "command.h"

struct scommand_s {
    GSList* coms;
    char* outputf;
    char* inputf;
    unsigned int length;
};

scommand scommand_new(void) {
    scommand result = malloc(sizeof(struct scommand_s));
    assert(result != NULL);
    result->coms = NULL;
    result->outputf = NULL;
    result->inputf = NULL;
    result->length = 0;

    return result;
}

scommand scommand_destroy(scommand self) {
    assert(self != NULL);
    
    g_slist_free_full(self->coms, g_free);
    free(self->outputf);
    free(self->inputf);
    free(self);
    self = NULL;

    return self;
}

bool scommand_is_empty(const scommand self) {
    assert(self != NULL);
    return (self->length == 0);
}

void scommand_push_back(scommand self, char * argument) {
    assert(self != NULL && argument != NULL);
    self->coms = g_slist_append(self->coms, argument);
    ++(self->length);
}

char * scommand_to_string(const scommand self) {
    assert(self != NULL); 

    unsigned int res_length = 0;

    // I create this array so I don't have to calculate the strlens twice
    unsigned int* coms_lengths = calloc(self->length, sizeof(unsigned int));

    GSList* com_elem = self->coms;
    for (unsigned int i = 0; i < self->length; ++i) {
        // save string length
        coms_lengths[i] = strlen((char*)com_elem->data);

        // + 1 for the SPACE char
        res_length += coms_lengths[i] + 1;
        com_elem = com_elem->next;
    }

    unsigned int inputf_length = 0;
    unsigned int outputf_length = 0;
    if (self->inputf) {
        inputf_length = strlen(self->inputf);

        // + 3 for the < and two SPACES
        res_length += inputf_length + 3;
    }

    if (self->outputf) {
        outputf_length = strlen(self->outputf);

        // + 3 for the > and two SPACES
        res_length += outputf_length + 3;
    }

    // if scommand has nothing return empty string
    if (res_length == 0) {
        free(coms_lengths);
        return calloc(1, sizeof(char));
    }
    
    char* res = calloc(res_length, sizeof(char));

    // copy_pos saves the position to copy in res
    unsigned int copy_pos = 0;
    com_elem = self->coms;
    for (unsigned int i = 0; i < self->length; ++i) {
        char* com_data = (char*)com_elem->data;
        strcpy(res + copy_pos, com_data);
        copy_pos += coms_lengths[i] + 1;
        res[copy_pos-1] = ' ';
        com_elem = com_elem->next;
    }

    if (self->inputf) {
        res[copy_pos] = '<';
        res[copy_pos + 1] = ' ';
        copy_pos += 2;
        strcpy(res + copy_pos, self->inputf);
        copy_pos += inputf_length + 1;
        res[copy_pos-1] = ' ';
    }

    if (self->outputf) {
        res[copy_pos] = '>';
        res[copy_pos + 1] = ' ';
        copy_pos += 2;
        strcpy(res + copy_pos, self->outputf);
        copy_pos += outputf_length + 1;
        res[copy_pos-1] = ' ';
    }

    res[copy_pos-1] = 0;

    free(coms_lengths);
    return res;
    
}

void scommand_pop_front(scommand self) {
    assert(self != NULL && !scommand_is_empty(self));

    gpointer killme_data = self->coms->data;
    self->coms = g_slist_remove(self->coms, killme_data);

    free(killme_data);
    --(self->length);
}

void scommand_set_redir_in(scommand self, char * filename) {
    assert(self != NULL);
    // free the previous inputf because scommand is the owner of the reference
    free(self->inputf);

    self->inputf = filename;
} 

void scommand_set_redir_out(scommand self, char * filename) {
    assert(self != NULL);
    // free the previous outputf because scommand is the owner of the reference
    free(self->outputf);

    self->outputf = filename;
}

unsigned int scommand_length(const scommand self) {
    assert(self != NULL);
    return self->length;
}


char * scommand_front(const scommand self) {
    assert(self != NULL && !scommand_is_empty(self));

    return self->coms->data;
}

char * scommand_get_redir_in(const scommand self){
	assert(self != NULL);
	
	return self->inputf;

}

char * scommand_get_redir_out(const scommand self){
	assert(self != NULL);
	
	return self->outputf;

}

struct pipeline_s{
    GSList *commands;  // List of commands
    bool fg; //Is it going to be run into the foreground, if its true you'll have to wait
    unsigned int length;
};


pipeline pipeline_new(void){
    pipeline result = malloc(sizeof(struct pipeline_s));
    result->commands = NULL;
    result->fg = true;
    result->length = 0;
    assert(result != NULL && pipeline_is_empty(result) && pipeline_get_wait(result));
    return result;
}

pipeline pipeline_destroy(pipeline self) {
    assert(self != NULL);
    GSList* list_elem = self->commands;
    while (list_elem) {
        GSList* kill_me = list_elem;
        scommand_destroy(list_elem->data);
        list_elem = list_elem->next;
        g_slist_free_1(kill_me);
    }
    free(self);
    return NULL;
}


bool pipeline_is_empty(const pipeline self) {
    assert(self != NULL);

    return self->length == 0;
}

bool pipeline_get_wait(const pipeline self) {
    assert(self != NULL);

    return self->fg;
}

void pipeline_push_back(pipeline self, scommand sc) {
    assert(self != NULL && sc != NULL);
    
    self->commands = g_slist_append(self->commands, sc);
    ++(self->length);
}

char *pipeline_to_string(const pipeline self) {
    assert(self != NULL);

    if (self->length == 0)
        return calloc(1, sizeof(char));
    
    // here I'll save each command string
    char** commands_strings = calloc(self->length, sizeof(char*));

    unsigned int res_length = 0;

    GSList* command = self->commands;
    for (unsigned int i = 0; i < self->length; ++i) {
        commands_strings[i] = scommand_to_string(command->data);
        command = command->next;
        res_length += strlen(commands_strings[i]);
    }

    // make room for & if ran in background
    res_length += !(self->fg) * 2;

    // *3 to make room for " | " that goes between each command
    // +1 to make room for the terminating null character
    char* res = calloc(res_length + (self->length-1) * 3 + 1, sizeof(char));

    for (unsigned int i = 0; i < self->length; ++i) {
        char* tmp = res;
        // merge res with command_string[i]
        res = strmerge(tmp, commands_strings[i]);
        // free previous tmp
        free(tmp);
        // if it is not the last command add " | "
        if (i != self->length - 1) {
            tmp = res;
            res = strmerge(tmp, " | ");
            free(tmp);
        } else if (!self->fg) {
            // add & if ran in background
            tmp = res;
            res = strmerge(tmp, " &");
            free(tmp);
        }
        free(commands_strings[i]);
    }

    free(commands_strings);
    return res;

}

void pipeline_pop_front(pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));

    scommand killme_command = self->commands->data;
    self->commands = g_slist_remove(self->commands, killme_command);

    scommand_destroy(killme_command);
    --(self->length);
}

void pipeline_set_wait(pipeline self, const bool w) {
    assert(self != NULL);
    
    self->fg = w;
}

unsigned int pipeline_length(const pipeline self) {
    assert(self != NULL);

    return self->length;
}

scommand pipeline_front(const pipeline self) {
    assert(self!= NULL && !pipeline_is_empty(self));

    return self->commands->data;
}
