#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      return -1;
    }
    sleep(&ticks, (struct spinlock *)0);
  }
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  xticks = ticks;
  return xticks;
}

//Turn of the computer
int
sys_halt(void){
  cprintf("Shutting down ...\n");
  outw( 0x604, 0x0 | 0x2000);
  return 0;
}

#ifdef CS333_P1
// Project 1 Implementation of the date() system call
int
sys_date(void)
{
  struct rtcdate *d;
  if(argptr(0, (void*)&d, sizeof(struct rtcdate)) < 0)
    return -1;
  else
    cmostime(d);
  return 0;
}
#endif
#ifdef CS333_P2
int
sys_getuid(void)
{
  return proc->uid;
}

int
sys_getgid(void)
{
  return proc->gid;
}

int
sys_getppid(void)
{
  if(proc->parent)
    return proc->parent->pid;
  else
    return -1;
}

int
sys_setuid(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1; // argint failed to fetch off the stack
  if(id < 0 || id >= 32768)
    return -1; // id is outside of acceptable range
  else
  {
    proc->uid = id;
    return 0;
  }
}

int
sys_setgid(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1; // argint failed to fetch off the stack
  if(id < 0 || id >= 32768)
    return -1; // id is outside of acceptale range
  else
  {
    proc->gid = id;
    return 0;
  }
}

int
sys_getprocs(void)
{
  struct uproc *up;
  int size;

  if(argint(0,&size) < 0 || argptr(1,(void*)&up, sizeof(*up) * size) < 0)
  {
    return -1;
  }

  return filluprocs(size, up);
}
#endif

#ifdef CS333_P3P4
int
sys_setpriority(void)
{
  int prio = 0;

  if(argint(1, &prio) < 0)
    return -1;
  if(prio < 0)  // Check for negative
    return -1;

  proc->priority = prio;
  return 0;
}
#endif
