#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{

  int pid;
for(int i = 0; i < 10; i++) {
  pid = fork();
  if(pid > 0) {
    // parent process
    pid = wait();
  } else if (pid == 0) {
    // child proces
    printf(1, "im child\n");
    sleep(2000);
  } else {
    printf(2, "fork error\n");
  }
}
  exit();
}
#endif
