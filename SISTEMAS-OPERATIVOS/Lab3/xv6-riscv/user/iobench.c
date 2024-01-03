#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

#define OPSIZE 160
#define TIMES 32
#define MINTICKS 100

static char path[] = "12iops";
static char data[OPSIZE];

static uint64 time(void) {
  uint64 n;
  __asm__ __volatile__ ("rdtime %0": "=r" (n));
  return n >> 20;
}

int
main(int argc, char *argv[])
{
  int rfd, wfd;
  int pid = getpid();
  int i;

  path[0] = '0' + (pid / 10);
  path[1] = '0' + (pid % 10);

  memset(data, 'a', sizeof(data));

  uint64 start = time();
  int opsw = 0,opsr = 0;
  uint64 total_ops=0;

  uint64 startglobal = time();
  uint64 endglobal = time();
  uint64 elapsedglobal = endglobal - startglobal;

  while(elapsedglobal< 2000) {
    uint64 end = time();
    uint64 elapsed = end - start;
    
    wfd = open(path, O_CREATE | O_WRONLY);
    
    for(i = 0; i < TIMES; ++i) {
      write(wfd, data, OPSIZE);
    }
        
    close(wfd);
    opsw += 2 * TIMES;

    rfd = open(path, O_RDONLY);

    for(i = 0; i < TIMES; ++i) {
      read(rfd, data, OPSIZE);
    }
    
    close(rfd);
    opsr += 2 * TIMES;

    if (elapsed >= MINTICKS) {
        printf("\t\t\t\t\t%d: %d OPW%dT, %d OPR%dT\n", pid
                      , (int) (opsw * MINTICKS / elapsed), MINTICKS
                      , (int) (opsr * MINTICKS / elapsed), MINTICKS);
  
        start = end;
        total_ops+=opsr+opsw;
        opsw = 0;
        opsr = 0;
        
    }
    
    endglobal = time();
    elapsedglobal = endglobal - startglobal;


  }
  printf("Termino iobench %d: total ops %lu -->\t",pid, total_ops);
  pstat(pid);
  exit(0);
}

