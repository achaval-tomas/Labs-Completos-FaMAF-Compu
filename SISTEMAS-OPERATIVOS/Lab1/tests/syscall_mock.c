#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "syscall_mock.h"

int mock_counter_open, mock_counter_close, mock_counter_dup,
    mock_counter_dup2, mock_counter_pipe, mock_counter_fork,
    mock_counter_execvp, mock_counter_exit, mock_counter_wait,
    mock_counter_waitpid, mock_counter_chdir;

/* Componentes para hacer mocks del sistema de file descriptor 
 * Esto es un poco más que un mock simple, sin conectarse a archivos externos
 * emula con bastante detalle el comportamiento de la tabla de descriptores
 * de un proceso UNIX.
 */

#define MOCK_FD_TABLE_SIZE 30 /* Tamaño de la tabla de falsos descriptores */
#define PIPE_NAME_LEN 3 /* Debe valer que MOCK_FD_TABLE_SIZE <= 10 ^ PIPE_NAME_LEN */

typedef struct {
    mock_file_kind kind;
    /* name es, segun kind:
     *  - NULL para archivos cerrados,
     *  - path si se abrio con open();
     *  - string con un entero para pipes. Los nombres de pipes se asignan
     *    desde "0" en orden creciente
     *  - string con nombre de dispositivo. typically "ttyin", "ttyout"
     */
    char *name; 
    bool readable, writable;
} mock_file;

static mock_file mock_fd_table[MOCK_FD_TABLE_SIZE];

static void mock_fd_clear (int fd) {
    /* Limpia un descriptor de archivo a estado cerrado*/
    assert (0<=fd && fd<MOCK_FD_TABLE_SIZE);
    mock_fd_table[fd].kind = KIND_CLOSED;
    free (mock_fd_table[fd].name); mock_fd_table[fd].name = NULL;
    mock_fd_table[fd].readable = false;
    mock_fd_table[fd].writable = false;
}

static int mock_fd_lookup (void) {
    /* Busca el primer descriptor libre. Devuelve -1 si estan todos usados */
    int result = 0;
    while (result<MOCK_FD_TABLE_SIZE && mock_fd_table[result].kind != KIND_CLOSED)
        result++;
    if (result >= MOCK_FD_TABLE_SIZE)
        result = -1;
    
    assert (result==-1 || mock_fd_table[result].kind == KIND_CLOSED);
    return result;
}

/* Setup del sistema de mock */

void mock_reset_all (void) {
    int i = 0;

    mock_counter_open = mock_counter_close = mock_counter_dup =
    mock_counter_dup2 = mock_counter_pipe = mock_counter_fork =
    mock_counter_execvp = mock_counter_exit = mock_counter_wait =
    mock_counter_waitpid = mock_counter_chdir = 0;
    if (mock_chdir_last!=NULL) {
        free (mock_chdir_last);
        mock_chdir_last = NULL;
    }
    mock_finished_processes_count = 0;
    /* Inicializar tabla de descriptores con un estado sensato:
     *  0 es el dispositivo "ttyin"
     *  1, 2 asociados a "ttyout"
     *  el resto de los descriptores cerrados
     */
    for (i=0; i< MOCK_FD_TABLE_SIZE; i++) {
        mock_fd_clear (i);
    }
    mock_fd_table[0].kind = KIND_DEV;
    mock_fd_table[0].name = strdup ("ttyin");
    mock_fd_table[0].readable = true;
    mock_fd_table[1].kind = KIND_DEV;
    mock_fd_table[1].name = strdup ("ttyout");
    mock_fd_table[1].writable = true;
    mock_fd_table[2].kind = KIND_DEV;
    mock_fd_table[2].name = strdup ("ttyout");
    mock_fd_table[2].writable = true;

}

static pid_t mock_fork_results[MAX_CHILDREN] = {-1};
static int mock_fork_results_index = 0;
void mock_fork_setup(pid_t results[]) {
    int i = 0, j = 0 ;
    
    mock_fork_results_index = 0;
    
    for (i=0;results[i]>0 && i<MAX_CHILDREN; i++) {
        mock_fork_results[i] = results[i];
    }
    /* Rellenar el resto de los resultados */
    for (j=i;j<MAX_CHILDREN; j++) {
        mock_fork_results[j] = results[i];
    }
}

static pid_t mock_waitable_processes[MAX_CHILDREN] = {-1};
void mock_wait_setup (pid_t pids[]) {
    int i = 0, j =0 ;
    
    for (i=0;pids[i]>0 && i<MAX_CHILDREN; i++) {
        mock_waitable_processes[i] = pids[i];
    }
    /* Rellenar el resto de los resultados */
    for (j=i;j<MAX_CHILDREN; j++) {
        mock_waitable_processes[j] = pids[i];
    }
}

/* Funciones para tests de operaciones de archivos: */

bool mock_check_fd (int fd, mock_file_kind kind, const char *name) {
    if (fd<0 || fd >= MOCK_FD_TABLE_SIZE) { /* Descriptor que no existe */
        return false;
    }
    return mock_fd_table[fd].kind == kind && ( /* El tipo coincide Y */
        /* El nombre esperado/existente es NULL */
        (mock_fd_table[fd].name == NULL && name == NULL) || 
        /* O coincide el nombre esperado con el existente */
        strcmp (mock_fd_table[fd].name, name) == 0
    );
}

bool mock_check_readable (int fd, bool expected){
    if (fd<0 || fd >= MOCK_FD_TABLE_SIZE) { /* Descriptor que no existe */
        return false;
    }
    return mock_fd_table[fd].readable == expected;
}

bool mock_check_writable (int fd, bool expected){
    if (fd<0 || fd >= MOCK_FD_TABLE_SIZE) { /* Descriptor que no existe */
        return false;
    }
    return mock_fd_table[fd].writable == expected;
}

/* Mocks */

int mock_open(const char *pathname, int flags, mode_t mode) {
    int fd = -1;
    mock_counter_open++;

    /* Si pathname es NULL, el error es EFAULT */
    if (pathname==NULL) {
        errno = EFAULT; return -1;
    }
    fd = mock_fd_lookup();
    if (fd >= 0 ) {
        int rwmode = flags & O_ACCMODE;
        mock_fd_table[fd].kind = KIND_OPEN;
        mock_fd_table[fd].name = strdup (pathname);
        mock_fd_table[fd].readable = rwmode == O_RDONLY || rwmode == O_RDWR;
        mock_fd_table[fd].writable = rwmode == O_WRONLY || rwmode == O_RDWR;
    } else {
        errno = EMFILE; /* Se lleno la tabla de descriptores */
    }
    return fd;
}

int mock_close (int fd) {
    int result = -1;
    mock_counter_close++;
    if (fd < 0 || fd >= MOCK_FD_TABLE_SIZE || mock_fd_table[fd].kind == KIND_CLOSED) {
        errno = EBADF; /* Número inválido, o ya estaba cerrado */
    } else {
        mock_fd_clear (fd);
        result = 0;
    }
    return result;
}

int mock_dup (int oldfd) {
    int result = -1;
    mock_counter_dup++;
    if (oldfd < 0 || oldfd >= MOCK_FD_TABLE_SIZE || mock_fd_table[oldfd].kind == KIND_CLOSED) {
        errno = EBADF; /* Número inválido, o ya estaba cerrado */
    } else {
        result = mock_fd_lookup ();
        if (result >= 0) {
            memcpy (mock_fd_table+result, mock_fd_table+oldfd, sizeof mock_fd_table[result] );
            /* Queremos un clon del string, no un alias: */
            mock_fd_table[result].name = strdup (mock_fd_table[result].name);
        } else {
            errno = EMFILE; /* Se acabaron los descriptores */
        }
    }
    return result;
}

int mock_dup2 (int oldfd, int newfd) {
    int result = -1;
    mock_counter_dup2++;
    if (oldfd < 0 || oldfd >= MOCK_FD_TABLE_SIZE || mock_fd_table[oldfd].kind == KIND_CLOSED) {
        errno = EBADF; /* Número inválido, o ya estaba cerrado */
    } else if (newfd < 0 || newfd >= MOCK_FD_TABLE_SIZE) {
        errno = EBADF; /* Número inválido */
    } else if (newfd == oldfd) {
        result = 0; /* Segun manpage, este caso es exitoso y no hace nada */
    } else {
        mock_fd_clear (newfd);
        result = newfd;
        memcpy (mock_fd_table+result, mock_fd_table+oldfd, sizeof mock_fd_table[result]);
        /* Queremos un clon del string, no un alias: */
        mock_fd_table[result].name = strdup (mock_fd_table[result].name);
    }
    return result;
}

int mock_pipe(int pipefd[2]) {
    int r_end = 0, w_end = 0, result = -1;
    mock_counter_pipe++;
    /* Si pipefd es NULL, el error es EFAULT */
    if (pipefd==NULL) {
        errno = EFAULT; return -1;
    }
    r_end = mock_fd_lookup ();
    if (r_end >= 0) {
        mock_fd_table[r_end].kind = KIND_PIPE;
        w_end = mock_fd_lookup ();
        if (w_end >= 0) {
            mock_fd_table[w_end].kind = KIND_PIPE;
            mock_fd_table[r_end].readable = true;
            mock_fd_table[w_end].writable = true;
            mock_fd_table[r_end].name = calloc (PIPE_NAME_LEN+1, 1);
            assert (mock_fd_table[r_end].name != NULL);
            sprintf (mock_fd_table[r_end].name, "%d", mock_counter_pipe-1);
            mock_fd_table[w_end].name = strdup (mock_fd_table[r_end].name);
            pipefd[0] = r_end;
            pipefd[1] = w_end;
            result = 0;
        } else {
            mock_fd_table[r_end].kind = KIND_CLOSED; /* Deshacer este cambio */
            errno = EMFILE; /* Se acabaron los descriptores */
        }
    } else {
        errno = EMFILE; /* Se acabaron los descriptores */
    }
    return result;
}

/* Process management mocks */
pid_t mock_fork (void) {
    int result = -1;
    mock_counter_fork++;
    if (mock_fork_results_index<MAX_CHILDREN) {
        result = mock_fork_results[mock_fork_results_index];
        mock_fork_results_index++;
    }
    if (result == -1) {
        errno = EAGAIN;
    }
    return result;
}

/* Estas variables se usan por el macro EXIT_PROTECTED. No declaradas como static
 * para que el macro (necesariamente en el .h) pueda accederlas
 */
bool _protected = false;
jmp_buf _exit_context;

const char *mock_execvp_last_file = NULL;
char *const *mock_execvp_last_argv = NULL;
int mock_execvp (const char *file, char *const argv[]) {
    mock_counter_execvp++;
    if (_protected) {
        /* No hace falta hacer copias. Como la ejecución termina en este
         * momento, nadie nos va a liberar esta memoria.
         * De hecho van a ser leaks, y no queda otra ya que el programa
         * original tampoco tenía como liberarlos, y no importaba por que
         * el exec() real pisa la imagen de memoria.
         */
        mock_execvp_last_file=file;
        mock_execvp_last_argv=argv;
        longjmp (_exit_context, 2);
    }
    errno = ENOEXEC;
    return -1;
}


int mock_exit_last = 0;
void mock_exit (int status) {
    mock_counter_exit++;
    mock_exit_last = status;
    /* Si no se hace un exit dentro de EXIT_PROTECTED, este mock va a hacer
     * cualquier cosa; el llamador muy probableme esta contando con el hecho de
     * que exit() detiene el flujo de control.
     * Así que si no salimos por las buenas (longjmp), salimos por las malas
     * (assert);
     */
    assert (_protected); 
    longjmp (_exit_context, 1);
    return;
}

pid_t mock_finished_processes[MAX_CHILDREN] = {0};
int mock_finished_processes_count = 0;

static void finish_process (int i) {
    int j = 0;
    /* Elimina el i-esimo elemento de la tabla de esperables, y lo loguea */
    assert (0<=i && i < MAX_CHILDREN);
    /* Loguear resultado */
    if (mock_finished_processes_count<MAX_CHILDREN) {
        mock_finished_processes[mock_finished_processes_count] = mock_waitable_processes[i];
        mock_finished_processes_count++;
    }
    /* Eliminar del conjunto de terminables */
    for (j=i; j+1<MAX_CHILDREN;j++) {
        mock_waitable_processes[j] = mock_waitable_processes[j+1];
    }
    mock_waitable_processes[MAX_CHILDREN-1] = -1;
}

pid_t mock_wait (int *status) {
    pid_t result = 0;

    mock_counter_wait++;
    
    if (mock_waitable_processes[0] > 0) { /* Hay un proceso para devolver */
        result = mock_waitable_processes[0];
        finish_process(0);
        /* Devolver estado de terminación, siempre 0*/
        if (status!= NULL)
            *status = 0;
    } else {
        errno = ECHILD;
        result = -1;
    }
    return result;
}

pid_t mock_waitpid (pid_t pid, int *status, int options) {
    pid_t result = 0;
    int i = 0;

    assert (pid==-1 || pid > 0); /* No sabemos hacer mock de otros casos*/

    mock_counter_waitpid++;

    if (pid==-1) { /* Comportamiento como el de wait */
        result = mock_wait (status);
        mock_counter_wait--; /*Pero no queremos tocar el contador */
    } else {
        while (i<MAX_CHILDREN && mock_waitable_processes[i]!=pid) {
            i++;
        }
        if (i<MAX_CHILDREN) { /* encontramos al proceso que buscabamos */
            result = mock_waitable_processes[i];
            finish_process (i);
            /* Devolver estado de terminación, siempre 0*/
            if (status!= NULL)
                *status = 0;
        } else {
            result= -1;
            errno = ECHILD;
        }
    }
    return result;
}

char* mock_chdir_last = NULL;
int mock_chdir (const char *path) {
    assert (path != NULL);
    mock_counter_chdir++;
    mock_chdir_last = strdup(path);
    errno = ENOENT;
    return -1;
}

