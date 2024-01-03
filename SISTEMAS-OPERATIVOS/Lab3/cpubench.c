#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

#define N 128
#define TIMES 32
#define MINTICKS 100

static float a[N][N];
static float b[N][N];
static float c[N][N];

static uint64 time(void) {
  uint64 n;
  __asm__ __volatile__ ("rdtime %0": "=r" (n));
  return n >> 20;
}

static void init(void) {
  int x, y;
  for (y = 0; y < N; ++y) {
    for (x = 0; x < N; ++x) {
      a[y][x] = y - x;
      b[y][x] = x - y;
      c[y][x] = 0.0f;
    }
  }
}

static void matmul(float beta) {
  int x, y, k;
  for (y = 0; y < N; ++y) {
    for (x = 0; x < N; ++x) {
      for (k = 0; k < N; ++k) {
        c[y][x] += beta * a[y][k] * b[k][x];
      }
    }
  }
}

int
main(int argc, char *argv[])
{
  int pid = getpid();
  float beta = 1.0f;
  
  init();
  uint64 start = time();
  uint64 startglobal = time();
  uint64 endglobal = time();
  uint64 elapsedglobal = endglobal - startglobal;
 
  uint64 ops = 0;
  uint64 total_ops = 0;
  while(elapsedglobal< 2000) {
    uint64 end = time();
    uint64 elapsed = end - start;
    if (elapsed >= MINTICKS) {
        int measurement = (ops * MINTICKS / elapsed) / 1000000;
        printf("%d: %d MFLOP%dT\n", pid, measurement, MINTICKS);

        start = end;
        total_ops+=ops;
        ops = 0;
    }

    for(int i = 0; i < TIMES; ++i) {
        matmul(beta);
        beta = -beta;
        ops += 3 * N * N * N;
    }
    endglobal = time();
    elapsedglobal = endglobal - startglobal;

  }

  printf("Termino cpubench %d: total ops %lu --> ",pid, total_ops);
  pstat(pid);
  exit(0);
}

