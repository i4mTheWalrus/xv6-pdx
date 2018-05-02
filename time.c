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

  ticks_in = uptime(); // mark when entering cpu

  pid = fork();

  if(pid > 0) {
    wait();
  } else if(pid == 0) {
    char* args[argc-1];
    for(int i = 0; i < argc; i++) {
      args[i] = malloc(sizeof(argv[i+1]));
      args[i] = argv[i+1];
    }
    exec(args[0], args);
  } else {
    printf(2, "fork error.");
  }

  ticks_out = uptime() - ticks_in;

  if(argc <= 1) {
    printf(1, " ran in ");
    printelapsed(ticks_out);
    printf(1, " seconds.\n");
  } else {
    printf(1, "%s ran in ", argv[1]);
    printelapsed(ticks_out);
    printf(1, " seconds.\n");
  }
  exit();
}

#endif
