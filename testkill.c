#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{

  int pid;
  pid = fork();
  if(pid > 0) {
    // parent process
    sleep(100);
    printf(1, "parent sleeping for 3 seconds\n");
    sleep(4000);
    printf(1, "parent attempting to kill child\n");
    kill(pid);
    printf(1, "done. Check zombie list\n");
    sleep(2000);
  } else if (pid == 0) {
    // child proces
    printf(1, "Child sleeping for 5 seconds\n");
    sleep(7000);
    printf(1, "Child exiting, wasn't killed.\n");
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
