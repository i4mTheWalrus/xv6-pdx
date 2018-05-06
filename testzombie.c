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
    printf(1, "parent sleeping for 3 seconds\n");
    sleep(6000);
    printf(1, "parent attempting to kill child\n");
    wait();
    printf(1, "done.\n");
  } else if (pid == 0) {
    // child proces
    sleep(100);
    printf(1, "Child sleeping for 5 seconds\n");
    sleep(3000);
    printf(1, "Child exiting, wasn't killed.\n");
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
