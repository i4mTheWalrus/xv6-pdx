#ifdef CS333_P2

#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  uint id = getppid();
  printf(1, "PPID: %d\n", id);
  exit();
}
#endif
