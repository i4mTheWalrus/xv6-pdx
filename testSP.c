#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "param.h"

int
main(int argc, char *argv[])
{
  int pid;
  for(int i = 0; i < 6; i++) {
    pid = fork();
    if(pid == 0)
      goto run;
    }

run:
  if(pid > 0) {
    // parent process. wait fo rthe children to fall in priority then change it
    sleep(3000);
    printf(1, "setting priority of a few processes.\n");
    pid = getpid() + 1;
    for(int j = pid; j < 2; j++) {
      setpriority(pid, 0);
    }
  } else if (pid == 0) {
    // child process
    while(1);
    exit();
  } else {
    printf(2, "fork error\n");
  }

  exit();
}
#endif
