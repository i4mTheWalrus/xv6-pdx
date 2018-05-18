#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "param.h"

int
main(int argc, char *argv[])
{
int pid;
for(int i = 0; i < 10; i++) {
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
    wait();
  } else if (pid == 0) {
    // child proces
    printf(1, "child entering infinite loop with lowest priority\n");

    // uncomment below to create the children with the lowest priority
    setpriority(getpid(), MAXPRIO);

    while(1);
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
