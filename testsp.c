#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "param.h"

int
main(int argc, char *argv[])
{

  int pid;
  pid = fork();
  pid = fork();
  if(pid > 0) {
    // parent process
    setpriority(getppid(), MAXPRIO);
    while(1) {};
  } else if (pid == 0) {
    // child proces
    setpriority(getppid(), MAXPRIO);
    while(1) {};
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
