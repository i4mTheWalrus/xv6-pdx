#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "param.h"

int
main(int argc, char *argv[])
{

  int pid;
  pid = fork();
  if(pid > 0) {
    // parent process
    sleep(50);
    printf(1, "parent going to sleep with max prio\n");
    setpriority(getppid(), MAXPRIO);
    sleep(99999);
  } else if (pid == 0) {
    // child proces
    printf(1, "child entering infinite loop with max prio\n");
    setpriority(getppid(), MAXPRIO);
    while(1) {};
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
