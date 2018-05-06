#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
int pid;
for(int i = 0; i < 18; i++) {
  pid = fork();
  if(pid == 0)
    goto run;
  }
//  int pid;
//  pid = fork();
//
run:
  if(pid > 0) {
    // parent process
    printf(1, "Note: children run forever, must kill qemu.\n", pid);
    wait();
  } else if (pid == 0) {
    // child proces
    printf(1, "child entering infinite loop\n");
    while(1);
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
