#ifndef SYSCALL_MOCK_H
#define SYSCALL_MOCK_H

#include <stdbool.h>
#include <sys/types.h>
#include <setjmp.h>

/* 
 * Reinicia todos los contadores del modulo de mock
 */
void mock_reset_all (void);


/* Distintas cosas a las que puede estar conectado un file descriptor */
typedef enum { KIND_CLOSED, KIND_OPEN, KIND_PIPE, KIND_DEV} mock_file_kind;

/*
 * Chequea que un filedescriptor este conectado a lo que se especifica
 * Es decir, que este conectado a un descriptor de tipo `kind' y a algo con
 * nombre `name'.
 *  Los valores de `name' pueden ser:
 *  - NULL para archivos cerrados,
 *  - path si se abrio con open();
 *  - string con un entero para pipes. Los nombres de pipes se asignan
 *    desde "0" en orden creciente
 *  - string con nombre de dispositivo. typically "ttyin", "ttyout"
 */
bool mock_check_fd (int fd, mock_file_kind kind, const char *name);

/*
 * Chequea que un filedescriptor sea legible/no legible (según expected)
 */
bool mock_check_readable (int fd, bool expected);

/*
 * Chequea que un filedescriptor sea escribible/no escribible (según expected)
 */
bool mock_check_writable (int fd, bool expected);

/* Máxima cantidad de resultados programables para fork/wait */
#define MAX_CHILDREN 10 

/*
 * Preprograma los resultados que va a ir devolviendo fork. El arreglo debe
 * terminar en un -1 (que es incluido en los resultados). No se pueden
 * programar mas de MAX_CHILDREN resultados; luego de esa cantidad de
 * llamadas, fork devuelve siempre -1.
 */
void mock_fork_setup (pid_t results[]);

/*
 * Preprograma los hijos que van a ir terminando y que wait/waitpid van a
 * encontrar. El arreglo debe terminar en un -1. No se pueden
 * programar mas de MAX_CHILDREN resultados; luego de esa cantidad de
 * llamadas, wait devuelve siempre -1.
 */
void mock_wait_setup (pid_t pids[]);

/*
 * No usar _mock_save_state directamente
 * El macro EXIT_PROTECTED(x), donde x es un bloque de código, ejecuta x hasta
 * que x termina, o hasta que x llama a las system calls exit() o exec().
 * Con éste se puede testear código que debería terminar el proceso
 * testeado.
 * Definido como macro ya que setjmp no se puede llamar desde una función.
 */
#define EXIT_PROTECTED(x) do{_protected= (setjmp (_exit_context)==0); if (_protected) {x}} while (0)
/* Estas variables son de uso privado; se declara en el .h porque necesitan ser
 * usadas por el macro anterior. No modificar directamente.
 */
extern bool _protected;
extern jmp_buf _exit_context;

/*
 * mock_xxx es la funcion que hace mock a la syscall xxx.
 * En cada una se detalla el comportamiento del mock
 */
int mock_open (const char *pathname, int flags, mode_t mode);
int mock_close (int fd);
int mock_dup (int oldfd);
int mock_dup2 (int oldfd, int newfd);
int mock_pipe (int pipefd[2]);

/*
 * Mock para fork. Devuelve en orden los resultados preprogramados con
 * mock_fork_setup. Cuando se agotan, devuelve -1. Cuando devuelve -1 setea
 * errno a EAGAIN.
 */
pid_t mock_fork (void);

/*
 * Mock para execvp.
 * Si no se usa dentro de un EXIT_PROTECTED falla con errno=ENOEXEC.
 * Dentro de un EXIT_PROTECTED termina.
 * Loguea sus argumentos en mock_execvp_last_file, mock_execvp_last_argv
 */
int mock_execvp (const char *file, char *const argv[]);
extern const char *mock_execvp_last_file;
extern char *const *mock_execvp_last_argv;

/*
 * Mock para exit. Guarda el status en mock_exit_last. Solo tiene sentido usar
 * este mock dentro de un bloque EXIT_PROTECTED; sino aborta, para evitar que
 * el llamador siga ejecutando detrás de un exit().
 */
void mock_exit (int status);
extern int mock_exit_last;

/*
 * Mock para wait. Devuelve el primer resultado de los programados con 
 * mock_wait_setup que no haya sido usado aun. Cuando se agotan, devuelve -1.
 * Cuando devuelve -1 setea errno a ECHILD. Sino siempre setea el status a 0.
 *
 * Los procesos que van siendo esperados se loguean en un arreglo
 * mock_finished_processes que tiene mock_finished_process_count elementos.
 * Este arreglo/contador se resetean com mock_reset_all.
 */
pid_t mock_wait (int *status);
/*
 * Mock para wait. Devuelve un resultado de los programados con 
 * mock_wait_setup que no haya sido usado aún. Cuando se agotan, devuelve -1.
 * Cuando devuelve -1 setea errno a ECHILD. Sino siempre setea el status a 0.
 * Ignora options. Solo acepta pid==-1, o pid>0. El orden de los pids devueltos
 * trata de simular el comportamiento original de waitpid.
 *
 * Los procesos que van siendo esperados se loguean en un arreglo
 * mock_finished_processes que tiene mock_finished_processes_count elementos.
 * Este arreglo/contador se resetean com mock_reset_all.
 */
pid_t mock_waitpid (pid_t pid, int *status, int options);
extern pid_t mock_finished_processes[MAX_CHILDREN];
extern int mock_finished_processes_count;

/*
 * Mock para chdir. Siempre falla con errno=ENOENT
 * Guarda una copia del último argumento en mock_chdir_last;
 */
int mock_chdir (const char *path);
extern char* mock_chdir_last;

/*
 * Para cada syscall hay un contador mock_counter_xxx indicando cuantas veces
 * fue invocada. Este contador se resetea a 0 con mock_reset_all()
 */
extern int mock_counter_open, mock_counter_close, mock_counter_dup,
	mock_counter_dup2, mock_counter_pipe, mock_counter_fork,
	mock_counter_execvp, mock_counter_exit, mock_counter_wait,
	mock_counter_waitpid, mock_counter_chdir;

#ifdef REPLACE_SYSCALLS

#define open mock_open
#define close mock_close
#define dup mock_dup
#define dup2 mock_dup2
#define pipe mock_pipe
#define fork mock_fork
#define execvp mock_execvp
#define exit mock_exit
#define wait mock_wait
#define waitpid mock_waitpid
#define chdir mock_chdir

#endif

#endif
