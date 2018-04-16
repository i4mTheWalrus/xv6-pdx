#ifdef CS333_P2

#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  uint id = getgid();
  printf(1, "GID: %d\n", id);
  exit();
}
#endif
