#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "param.h"

int
main(int argc, char *argv[])
{
  int pid;
  for(int i = 0; i < 10; i++) {
    pid = fork();
    if(pid == 0)
      goto run;
  }

run:
  if(pid > 0) {
    // parent process
    wait();
  } else if (pid == 0) {
    // child process
    while(1);
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
