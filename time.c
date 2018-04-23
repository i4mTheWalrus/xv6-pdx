#ifdef CS333_P2
#include "types.h"
#include "user.h"

#define MAX_CHARS  64

// Helper function to print a floating point looking number
static void
printelapsed(uint ticks)
{
  printf(1, "%d", ticks/1000);
  printf(1, ".%d", ticks % 1000);
}

int
main(int argc, char *argv[])
{
  int ticks_in = 0, ticks_out = 0;
  int pid;

  // only move ahead if there are args
  if(argc > 1)
  {
    ticks_in = uptime(); // mark when entering cpu

    pid = fork();
    if(pid > 0) {
      // PARENT PROCESS
      pid = wait();
      printf(1, "%s ran in ", argv[1]);
      printelapsed(ticks_out);
      printf(1, " seconds.\n");
    } else if (pid == 0) {
      // CHILD PROCESS
      // copy the arg list into a new one to execute with
      char** newargs = malloc(sizeof(*newargs) * (argc+1));
      for(int i = 0; i < argc; i++) // allocate memory
      {
        newargs[i] = malloc(MAX_CHARS);
        newargs[i] = argv[i+2];
      }
      newargs[argc] = 0;
      exec(argv[1], newargs);
    } else {
      printf(2, "fork error\n");
    }
    // mark when leaving the cpu
    ticks_out = uptime() - ticks_in;
  }
  else { // time was called without an arg
    printf(1, " ran in ");
    printelapsed(ticks_out);
    printf(1, " seconds.\n");
  }

  exit();
}

#endif
