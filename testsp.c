#ifdef CS333_P2
#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "setting priority to 2\n");
  setpriority(getpid(), 2);
  sleep(5000);
//  printf(1, "setting priority to 0\n");
//  setpriority(getpid(), 0);
  exit();
}
#endif
