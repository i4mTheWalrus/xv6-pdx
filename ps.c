#ifdef CS333_P2
#include "types.h"
#include "user.h"
#include "uproc.h"

static uint MAX_UPROCS = 16; // maximum number of entries in urpoc table

// Helper function to print a floating point looking number
static void
printelapsed(uint ticks)
{
  if(ticks >= 1000)
  {
    printf(1, "%d", ticks/1000);
    printf(1, ".%d", ticks % 1000);
  }
  else {
    printf(1, "%d", ticks/100);
    printf(1, ".%d", ticks % 100);
  }
}

int
main(void)
{
  // malloc an array for the process table
  // fill the table with getprocs(max,uproc)
  // print stuff about the procs
  uint max = MAX_UPROCS;
  struct uproc **uptable = malloc(MAX_UPROCS * sizeof(struct uproc));

  for(int i = 0; i < MAX_UPROCS; i++)
  {
    uptable[i] = malloc(sizeof(struct uproc));
  }

  int uprocnum = getprocs(max, uptable);
  if(uprocnum <= 0)
    exit();

  // print a header
  printf(1, "PID\tName\tUID\tGID\tPPID\tElapsed\tCPU\tState\tSize\n");

  // print the uprocs
  for(int i = 0; i < uprocnum; i++)
  {
    printf(1, "%d\t%s\t%d\t%d\t%d\t",
           uptable[i]->pid, uptable[i]->name, uptable[i]->uid,
           uptable[i]->gid, uptable[i]->ppid);
    printelapsed(uptable[i]->elapsed_ticks);
    printf(1, "\t");
    printelapsed(uptable[i]->CPU_total_ticks);
    printf(1, "\t%s\t%d\t\n", uptable[i]->state, uptable[i]->size);
  }

  free(uptable);
  exit();
}
#endif
