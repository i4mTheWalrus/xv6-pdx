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
  } else if (pid == 0) {
    // child proces
    printf(1, "Child exiting, parent still waiting.\n");
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
