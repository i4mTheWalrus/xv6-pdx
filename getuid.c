#ifdef CS333_P2

#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  uint id = getuid();
  printf(1, "UID: %d\n", id);
  exit();
}
#endif
