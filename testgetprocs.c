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
    // child process
    exec(argv[1], argv);
  } else {
    printf(2, "fork error\n");
  }
  exit();
}
#endif
