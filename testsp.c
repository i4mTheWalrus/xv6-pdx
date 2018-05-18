#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{

  int pid;
  pid = fork();
  pid = fork();
  if(pid > 0) {
    // parent process
    while(1) {};
  } else if (pid == 0) {
    // child proces
    while(1) {};
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
