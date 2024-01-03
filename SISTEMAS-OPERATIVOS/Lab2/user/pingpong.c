#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int 
main(int argc, char** argv) 
{

    if (argc != 2) {
        fprintf(2, "ERROR: pingpong accepts a single argument, the amount of rounds.\n");
        return 1;
    }

    int rounds = atoi(argv[1]);

    if (rounds <= 1) {
        fprintf(2, "ERROR: number of rounds must be greater than 1.\n");
        return 1;
    }
    
    int child_sem = get_sem(0);
    int father_sem = get_sem(1);

    if (child_sem == -1 || father_sem == -1){
        fprintf(2, "ERROR: No semaphores available.\n");
        return 1;
    }

    int rc = fork();

    if (rc == -1) {
        fprintf(2, "Error when forking.\n");
        return 1;
    } else if (rc == 0) {
        // child
        for (unsigned int i = 0; i < rounds; ++i) {
            sem_down(child_sem);
            printf("\tpong\n");
            sem_up(father_sem);
        }
        return 0;
    } else {
        // father
        for (unsigned int i = 0; i < rounds; ++i) {
            sem_down(father_sem);
            printf("ping\n");
            sem_up(child_sem);
        }
    }

    wait(0);
    sem_close(father_sem);
    sem_close(child_sem);

  
    return 0;
}
