#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "setting priority to 2\n");
  setpriority(getpid(), 2);
  sleep(5000);
  exit();
}
#endif
