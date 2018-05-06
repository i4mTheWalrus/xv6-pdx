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
    printf(1, "parent waiting\n");
    wait();
  } else if (pid == 0) {
    // child proces
    sleep(100);
    printf(1, "Child sleeping for 5 seconds\n");
    sleep(7000);
    printf(1, "Child exiting.\n");
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
