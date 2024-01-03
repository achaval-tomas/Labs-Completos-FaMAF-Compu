#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
  int pid = getpid();

  pstat(pid);

  return 0;
}
