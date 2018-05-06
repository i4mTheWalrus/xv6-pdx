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
    pid = wait();
  } else if (pid == 0) {
    // child proces
    printf(1, "Child spawned, waiting a short time. Press crtl-f\n");
    sleep(2000);
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
