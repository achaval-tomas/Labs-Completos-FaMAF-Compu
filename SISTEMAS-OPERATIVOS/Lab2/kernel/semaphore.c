#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

#define MAX_SEM 256

struct semaphorius {
    int value;           // normal value >= 0, 
    struct spinlock lock;
};

struct semaphorius sem_arr[MAX_SEM]; // global var for kernel


void
sem_init_arr() {
    for (unsigned int i = 0; i < MAX_SEM; ++i) {
        sem_arr[i].value = -1;
        initlock(&(sem_arr[i].lock), "semaphore");
    }
}

/* you put a good fight my dear, im gonna miss you :( Goodbye my lovargh
int sem_is_used(int sem) {

    acquire(&(sem_arr[sem].lock));

    int is_used = (sem_arr[sem].value) != -1;

    release(&(sem_arr[sem].lock));

    return is_used;
} 
    god gives its hardest battles to its toughest functions...
*/

/*
    Iterates through semaphores until one is available.
    Opens it initializing it with value, and returns its ID. Otherwise, returns -1.
*/
int
get_sem(int value)
{
    int sem = 0;

    while (sem < MAX_SEM && !sem_open(sem, value)) {
        ++sem;
    }

    return (sem == MAX_SEM) ? -1 : sem;
}

/*
    If sem was already open, it leaves its value as it was and returns 0.
    If sem was closed, sets its value and returns 1.
*/
int
sem_open(int sem, int value)
{   
    if (sem >= MAX_SEM || sem < 0) {
        printf("ERROR: invalid sem ID.\n");
        return 0;
    }

    char ret_value = 0;
    
    acquire(&(sem_arr[sem].lock));

    if (sem_arr[sem].value == -1) {
        sem_arr[sem].value = value;
        ret_value = 1;
    }

    release(&(sem_arr[sem].lock));

    return ret_value;
}

int
sem_close(int sem)
{
    
    if (sem >= MAX_SEM || sem < 0) {    
        printf("ERROR: invalid sem ID.\n");
        return 0;
    }

    acquire(&(sem_arr[sem].lock));

    sem_arr[sem].value = -1;
    
    release(&(sem_arr[sem].lock));
    return 1;
}

int
sem_up(int sem)
{
    
    acquire(&(sem_arr[sem].lock));

    if (sem_arr[sem].value == -1) {
        printf("ERROR: tried to increase closed semaphore.\n");
        release(&(sem_arr[sem].lock));
        return 0;
    }
   
    if (sem_arr[sem].value == 0)
        wakeup(&(sem_arr[sem]));
    
    ++(sem_arr[sem].value);

    release(&(sem_arr[sem].lock));

    return 1;
}

int
sem_down(int sem)
{
    acquire(&(sem_arr[sem].lock));

    if (sem_arr[sem].value == -1) {
        printf("ERROR: tried to decrease closed semaphore.\n");
        release(&(sem_arr[sem].lock));
        return 0;
    }

    while (sem_arr[sem].value == 0) {
        sleep(&(sem_arr[sem]), &(sem_arr[sem].lock));
    }
    --(sem_arr[sem].value);

    release(&(sem_arr[sem].lock));
    
    return 1;
}
