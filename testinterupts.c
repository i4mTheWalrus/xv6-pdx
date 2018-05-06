#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "forking twice and sleeping all 4 processes. test ctrl-s and ctrl-f\n");
  fork();
  fork();
  sleep(5000);

  exit();
}
#endif
