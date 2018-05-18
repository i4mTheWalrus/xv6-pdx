#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

#ifdef CS333_P3P4
struct StateLists {
  struct proc* ready[MAXPRIO + 1];
  struct proc* readyTail[MAXPRIO + 1];
  struct proc* free;
  struct proc* freeTail;
  struct proc* sleep;
  struct proc* sleepTail;
  struct proc* zombie;
  struct proc* zombieTail;
  struct proc* running;
  struct proc* runningTail;
  struct proc* embryo;
  struct proc* embryoTail;
};

struct {
  uint PromoteAtTime;
  struct spinlock lock;
  struct proc proc [NPROC];
  struct StateLists pLists;
} ptable ;

#else
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;
#endif

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

#ifdef CS333_P3P4
static void initProcessLists(void);
static void initFreeList(void);
static int stateListAdd(struct proc** head, struct proc** tail, struct proc* p);
static int stateListRemove(struct proc** head, struct proc** tail, struct proc* p);

static void
assertState(struct proc *p, enum procstate state) {
  if(!p)
    panic("proc unallocated.\n");
  if(p->state != state) {
    cprintf("p->pid = %d, p->name = %s, p->state = %d, state = %d\n", p->pid, p->name, p->state, state);
    panic("state assertion failed.\n");
  }
}

static void
assertPrio(struct proc *p, uint prio) {
  if(!p)
    panic("proc unallocated.\n");
  if(p->priority != prio) {
    cprintf("p->pid = %d, p->name = %s, p->prio = %d, prio = %d\n", p->pid, p->name, p->priority, prio);

    panic("priority assertion failed.\n");
  }
}
#endif

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

#ifdef CS333_P3P4
  acquire(&ptable.lock);
  p = ptable.pLists.free; // set p to first node in free list
  if(p)
    goto found;

  release(&ptable.lock);
  return 0;
#endif

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);

  return 0;

found:
#ifdef CS333_P3P4
  stateListRemove(&ptable.pLists.free, &ptable.pLists.freeTail, p);
  assertState(p, UNUSED);
  p->state = EMBRYO;
  stateListAdd(&ptable.pLists.embryo, &ptable.pLists.embryoTail, p);
  p->pid = nextpid++;
  release(&ptable.lock);
#else
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);
#endif

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
#ifdef CS333_P3P4
    acquire(&ptable.lock);
    stateListRemove(&ptable.pLists.embryo, &ptable.pLists.embryoTail, p);
    assertState(p, EMBRYO);
    p->state = UNUSED;
    stateListAdd(&ptable.pLists.free, &ptable.pLists.freeTail, p);
    release(&ptable.lock);
    return 0;
#else
    p->state = UNUSED;
    return 0;
#endif
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

#ifdef CS333_P1
  p->start_ticks = ticks;
#endif
#ifdef CS333_P2
  p->cpu_ticks_total = 0;
  p->cpu_ticks_in = 0;
#endif
#ifdef CS333_P3P4
  p->priority = 0;  // procs start at highest prio
  p->budget = BUDGET;
#endif

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

#ifdef CS333_P3P4
  acquire(&ptable.lock);
  initProcessLists();
  initFreeList();
  ptable.PromoteAtTime = ticks + TICKS_TO_PROMOTE;
  release(&ptable.lock);
#endif

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

#ifdef CS333_P2
  // Initialize gid and uid values
  p->uid = STARTUID;
  p->gid = STARTGID;
#endif
#ifdef CS333_P3P4
  acquire(&ptable.lock);
  assertState(p, EMBRYO);
  stateListRemove(&ptable.pLists.embryo, &ptable.pLists.embryoTail, p);
  p->state = RUNNABLE;
  p->next = 0;
  stateListAdd(&ptable.pLists.ready[0], &ptable.pLists.readyTail[0], p);
  ptable.pLists.sleep = 0;
  ptable.pLists.sleepTail = 0;
  ptable.pLists.zombie = 0;
  ptable.pLists.zombieTail = 0;
  ptable.pLists.running = 0;
  ptable.pLists.runningTail = 0;
  ptable.pLists.embryo = 0;
  ptable.pLists.embryoTail = 0;
  release(&ptable.lock);
#else
  p->state = RUNNABLE;
#endif
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;

  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  proc->sz = sz;
  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
#ifdef CS333_P3P4
    acquire(&ptable.lock);
    stateListRemove(&ptable.pLists.embryo, &ptable.pLists.embryoTail, np);
    np->state = UNUSED;
    stateListAdd(&ptable.pLists.free, &ptable.pLists.freeTail, np);
    release(&ptable.lock);
#else
    np->state = UNUSED;
#endif
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  safestrcpy(np->name, proc->name, sizeof(proc->name));

  pid = np->pid;

#ifdef CS333_P2
  // Initialize gid and uid values
  if(np->parent) {
    np->uid = np->parent->uid;
    np->gid = np->parent->gid;
  }
#endif

  // lock to force the compiler to emit the np->state write last.
#ifdef CS333_P3P4
  acquire(&ptable.lock);
  assertState(np, EMBRYO);
  stateListRemove(&ptable.pLists.embryo, &ptable.pLists.embryoTail, np);
  np->state = RUNNABLE;
  // Add to the highest priority queue
  stateListAdd(&ptable.pLists.ready[0], &ptable.pLists.readyTail[0], np);
  release(&ptable.lock);
#else
  acquire(&ptable.lock);
  np->state = RUNNABLE;
  release(&ptable.lock);
#endif
  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
#ifndef CS333_P3P4
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}
#else
void
exit(void) // P3 exit ****************
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(proc->cwd);
  end_op();
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to initproc
  if((p = ptable.pLists.running)) {
  while(p) {
    if(p->parent == proc) p->parent = initproc;
    p = p->next;
  }
  }

  for(int i = 0; i <= MAXPRIO; i++)
  {
    if((p = ptable.pLists.ready[i])) {
    while(p)
    {
      if(p->parent == proc) p->parent = initproc;
      p = p->next;
    }
    }
  }

  if((p = ptable.pLists.embryo)) {
  while(p) {
    if(p->parent == proc) p->parent = initproc;
    p = p->next;
  }
  }

  if((p = ptable.pLists.zombie)) {
  while(p) {
    if(p->parent == proc) {
      p->parent = initproc;
      wakeup1(initproc);
    }
    p = p->next;
  }
  }

  if((p = ptable.pLists.sleep)) {
  while(p) {
    if(p->parent == proc) p->parent = initproc;
    p = p->next;
  }
  }

  // Jump into the scheduler, never to return.
  assertState(proc, RUNNING);
  stateListRemove(&ptable.pLists.running, &ptable.pLists.runningTail, proc);
  proc->state = ZOMBIE;
  stateListAdd(&ptable.pLists.zombie, &ptable.pLists.zombieTail, proc);
  sched();
  panic("zombie exit");
}
#endif

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
#ifndef CS333_P3P4
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
#else
int
wait(void) // P3 wait **************************
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    if((p = ptable.pLists.zombie)) {
    while(p) {
      if(p->parent == proc) {
        // Found one.
        havekids = 1;
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        stateListRemove(&ptable.pLists.zombie, &ptable.pLists.zombieTail, p);
        p->state = UNUSED;
        stateListAdd(&ptable.pLists.free, &ptable.pLists.freeTail, p);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->priority = 0;
        release(&ptable.lock);
        return pid;
      }

      p = p->next;
    }
    }

    // Look through other lists for children
    // (mirroring prior implementation)
    for(int i = 0; i <= MAXPRIO; i++) {
      p = ptable.pLists.ready[i];
      while(p) {
        if(p->parent == proc) {
          havekids = 1;
          goto nowait;
        }
        p = p->next;
      }
    }

    if((p = ptable.pLists.sleep)){
    while(p) {
      if(p->parent == proc) {
        havekids = 1;
        goto nowait;
      }
      p = p->next;
    }
    }

    if((p = ptable.pLists.embryo)) {
    while(p) {
      if(p->parent == proc) {
        havekids = 1;
        goto nowait;
      }
      p = p->next;
    }
    }

    if((p = ptable.pLists.running)) {
    while(p) {
      if(p->parent == proc) {
        havekids = 1;
        goto nowait;
      }
      p = p->next;
    }
    }

nowait:
    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}
#endif

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
#ifndef CS333_P3P4
// original xv6 scheduler. Use if CS333_P3P4 NOT defined.
void
scheduler(void)
{
  struct proc *p;
  int idle;  // for checking if processor is idle

  for(;;){
    // Enable interrupts on this processor.
    sti();

    idle = 1;  // assume idle unless we schedule a process
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

#ifdef CS333_P2
      p->cpu_ticks_in = ticks;
#endif
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      idle = 0;  // not idle this timeslice
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);
    // if idle, wait for next interrupt
    if (idle) {
      sti();
      hlt();
    }
  }
}

#else
void
scheduler(void) // P3 scheduler ***************************
{
  struct proc *p;//, *np;
  int idle;  // for checking if processor is idle
  int i = 0;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    idle = 1;  // assume idle unless we schedule a process
    acquire(&ptable.lock);

    for(i = 0; i <= MAXPRIO; i++){
      p = ptable.pLists.ready[i];
      if(p)
        break;
    }

    if(p) {
      p->cpu_ticks_in = ticks;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      idle = 0;  // not idle this timeslice
      proc = p;
      switchuvm(p);
      assertState(p, RUNNABLE);
      assertPrio(p, i);
      stateListRemove(&ptable.pLists.ready[p->priority], &ptable.pLists.readyTail[p->priority], p);
      p->state = RUNNING;
      stateListAdd(&ptable.pLists.running, &ptable.pLists.runningTail, p);
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }

    // CHECK IF ITS TIME TO PROMOTE
    if(ticks >= ptable.PromoteAtTime) {
      struct proc *np;
      for(int i = 1; i <= MAXPRIO; i++) { // skip queue 0 (highest queue)
        p = ptable.pLists.ready[i];
        while(p) {
          np = p->next;
          assertPrio(p, i);
          stateListRemove(&ptable.pLists.ready[p->priority], &ptable.pLists.readyTail[p->priority], p);
          p->priority = p->priority - 1;
          p->budget = BUDGET;     // reset budget
          stateListAdd(&ptable.pLists.ready[p->priority], &ptable.pLists.readyTail[p->priority], p);
          p = np;
        }
      }
      ptable.PromoteAtTime = ticks + TICKS_TO_PROMOTE;
    }

    release(&ptable.lock);
    // if idle, wait for next interrupt
    if (idle) {
      sti();
      hlt();
    }
  }
}
#endif

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;

#ifdef CS333_P2
  proc->cpu_ticks_total += (ticks - proc->cpu_ticks_in);
#endif

  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
#ifdef CS333_P3P4
  acquire(&ptable.lock);  //DOC: yieldlock
  assertState(proc, RUNNING);
  stateListRemove(&ptable.pLists.running, &ptable.pLists.runningTail, proc);
  proc->state = RUNNABLE;
  proc->budget = proc->budget - (ticks - proc->cpu_ticks_in);
  if(proc->budget <= 0) {
    if(proc->priority < MAXPRIO) {
      proc->budget = BUDGET;
      proc->priority++;
    }
    else // already at lowest priority
      proc->budget = BUDGET;
  }
  stateListAdd(&ptable.pLists.ready[proc->priority], &ptable.pLists.readyTail[proc->priority], proc);
  sched();
  release(&ptable.lock);
#else
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
#endif
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// 2016/12/28: ticklock removed from xv6. sleep() changed to
// accept a NULL lock to accommodate.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){
    acquire(&ptable.lock);
    if (lk) release(lk);
  }

  // Go to sleep.
#ifdef CS333_P3P4
  proc->chan = chan;
  assertState(proc, RUNNING);
  stateListRemove(&ptable.pLists.running, &ptable.pLists.runningTail, proc);
  proc->budget = proc->budget - (ticks - proc->cpu_ticks_in);
  if(proc->budget <= 0) {
    if(proc->priority < MAXPRIO) {
      proc->budget = BUDGET;
      proc->priority++;
    }
    else // already at lowest priority
      proc->budget = BUDGET;
  }
  proc->state = SLEEPING;
  stateListAdd(&ptable.pLists.sleep, &ptable.pLists.sleepTail, proc);
#else
  proc->chan = chan;
  proc->state = SLEEPING;
#endif
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){
    release(&ptable.lock);
    if (lk) acquire(lk);
  }
}

//PAGEBREAK!
#ifndef CS333_P3P4
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}
#else
static void
wakeup1(void *chan) // *********** P3 ***************
{
  struct proc *p, *temp;

  p = ptable.pLists.sleep;
  while(p) {
    temp = p->next;
    if(p->chan == chan) {
      assertState(p, SLEEPING);
      stateListRemove(&ptable.pLists.sleep, &ptable.pLists.sleepTail, p);
      p->state = RUNNABLE;
      stateListAdd(&ptable.pLists.ready[p->priority], &ptable.pLists.readyTail[p->priority], p);
    }
    p = temp;
  }
}
#endif

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
#ifndef CS333_P3P4
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}
#else
int
kill(int pid) // ****** P3 ***********************
{
  struct proc *p;

  acquire(&ptable.lock);

  if((p = ptable.pLists.sleep)) {
    while(p) {
      if(p->pid == pid){
        p->killed = 1;
        assertState(p, SLEEPING);
        stateListRemove(&ptable.pLists.sleep, &ptable.pLists.sleepTail, p);
        p->state = RUNNABLE;
        // add a killed proc to highest priority
        stateListAdd(&ptable.pLists.ready[0], &ptable.pLists.readyTail[0], p);
        release(&ptable.lock);
        return 0;
      }
      p = p->next;
    }
  }

  for(int i = 0; i <= MAXPRIO; i++) {
    if((p = ptable.pLists.ready[i])) {
      while(p) {
        if(p->pid == pid) {
          p->killed = 1;
          release(&ptable.lock);
          return 0;
        }
        p = p->next;
      }
    }
  }

  if((p = ptable.pLists.running)) {
    while(p) {
      if(p->pid == pid) {
        p->killed = 1;
        release(&ptable.lock);
        return 0;
      }
      p = p->next;
    }
  }

  if((p = ptable.pLists.embryo)) {
    while(p) {
      if(p->pid == pid) {
        p->killed = 1;
        release(&ptable.lock);
        return 0;
      }
      p = p->next;
    }
  }

  release(&ptable.lock);
  return -1;
}
#endif

static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
};

#ifdef CS333_P1
// Helper function to print a floating point looking number
static void
printelapsed(uint ticks)
{
  cprintf("%d", ticks/1000);
  cprintf(".%d", ticks % 1000);
}

void
procdumpP1(struct proc *p, char *state)
{
  cprintf("%d\t%s\t", p->pid, p->name);
  printelapsed(ticks - p->start_ticks);
  cprintf("%s\t", state);
}
#endif

#ifdef CS333_P2
void
procdumpP2(struct proc *p, char *state)
{
  cprintf("%d\t%s\t%d\t%d\t", p->pid, p->name, p->uid, p->gid);
  if(p->pid == 1)
    cprintf("1\t");
  else
    cprintf("%d\t", p->parent->pid);
  printelapsed(ticks - p->start_ticks);
  cprintf("%s\t", "");
  printelapsed(p->cpu_ticks_total);
  cprintf("\t%s\t%d\t", state, p->sz);
}
#endif

#ifdef CS333_P3P4
void
procdumpP3P4(struct proc *p, char *state)
{
  cprintf("%d\t%s\t%d\t%d\t", p->pid, p->name, p->uid, p->gid);
  if(p->pid == 1)
    cprintf("1\t");
  else
    cprintf("%d\t", p->parent->pid);
  cprintf("%d\t", p->priority);
  printelapsed(ticks - p->start_ticks);
  cprintf("%s\t", "");
  printelapsed(p->cpu_ticks_total);
  cprintf("\t%s\t%d\t", state, p->sz);
}
#endif

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.

// Note code obtained from Mark Morrissey at PSU
void
procdump(void)
{
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

#if defined(CS333_P3P4)
#define HEADER "\nPID\tName\tUID\tGID\tPPID\tPrio\tElapsed\tCPU\tState\tSize\t PCs\n"
#elif defined(CS333_P2)
#define HEADER "\nPID\tName\tUID\tGID\tPPID\tElapsed\tCPU\tState\tSize\t PCs\n"
#elif defined(CS333_P1)
#define HEADER "\nPID\tName\tElapsed\tState\tSize\t PCs\n"
#else
#define HEADER ""
#endif

  cprintf(HEADER);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";

#if defined(CS333_P3P4)
  procdumpP3P4(p, state);
#elif defined(CS333_P2)
  procdumpP2(p, state);
#elif defined(CS333_P1)
  procdumpP1(p, state);
#else
  cprintf("%d %s %s", p->pid, state, p->name);
#endif

    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }

    cprintf("\n");
  }
}

#ifdef CS333_P3P4
static int
stateListAdd(struct proc** head, struct proc** tail, struct proc* p)
{
  if (*head == 0) {
    *head = p;
    *tail = p;
    p->next = 0;
  } else {
    (*tail)->next = p;
    *tail = (*tail)->next;
    (*tail)->next = 0;
  }

  return 0;
}

static int
stateListRemove(struct proc** head, struct proc** tail, struct proc* p)
{
  if (*head == 0 || *tail == 0 || p == 0) {
    return -1;
  }

  struct proc* current = *head;
  struct proc* previous = 0;

  if (current == p) {
    *head = (*head)->next;
    // prevent tail remaining assigned when we've removed the only item
    // on the list
    if(*tail == p){
      *tail = 0;
    }
    return 0;
  }

  while(current) {
    if (current == p) {
      break;
    }

    previous = current;
    current = current->next;
  }

  // Process not found, hit eject.
  if (current == 0) {
    return -1;
  }

  // Process found. Set the appropriate next pointer.
  if (current == *tail) {
    *tail = previous;
    (*tail)->next = 0;
  } else {
    previous->next = current->next;
  }

  // Make sure p->next doesn't point into the list.
  p->next = 0;

  return 0;
}

static void
initProcessLists(void) {
  for(int i = 0; i <= MAXPRIO; i++)
  {
    ptable.pLists.ready[i] = 0;
    ptable.pLists.readyTail[i] = 0;
  }
  ptable.pLists.free = 0;
  ptable.pLists.freeTail = 0;
  ptable.pLists.sleep = 0;
  ptable.pLists.sleepTail = 0;
  ptable.pLists.zombie = 0;
  ptable.pLists.zombieTail = 0;
  ptable.pLists.running = 0;
  ptable.pLists.runningTail = 0;
  ptable.pLists.embryo = 0;
  ptable.pLists.embryoTail = 0;
}

static void
initFreeList(void) {
  if (!holding(&ptable.lock)) {
    panic("acquire the ptable lock before calling initFreeList\n");
  }

  struct proc* p;

  for (p = ptable.proc; p < ptable.proc + NPROC; ++p) {
    p->state = UNUSED;
    stateListAdd(&ptable.pLists.free, &ptable.pLists.freeTail, p);
  }
}
#endif

#ifdef CS333_P2
int
filluprocs(uint max, struct uproc *uptable)
{
  // Note that this function assumes uptable has already been allocated
  struct proc *p;
  int i = 0; // uptable index and total

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if(i < max && p->state != UNUSED)
    {
      if(p->parent)
        uptable[i].ppid = p->parent->pid;
      else
        uptable[i].ppid = 1;
      uptable[i].pid = p->pid;
      uptable[i].uid = p->uid;
      uptable[i].gid = p->gid;
      uptable[i].priority = p->priority;
      uptable[i].elapsed_ticks = ticks - p->start_ticks;
      uptable[i].CPU_total_ticks = p->cpu_ticks_total;
      uptable[i].size = p->sz;
      safestrcpy(uptable[i].name, p->name, sizeof(p->name));

      switch(p->state) // check if this proc should be copied
      {
        case EMBRYO:
          safestrcpy(uptable[i].state, "EMBRYO", sizeof("EMBRYO"));
          break;
        case RUNNABLE:
          safestrcpy(uptable[i].state, "RUNNABLE", sizeof("RUNNABLE"));
          break;
        case SLEEPING:
          safestrcpy(uptable[i].state, "SLEEPING", sizeof("SLEEPING"));
          break;
        case RUNNING:
          safestrcpy(uptable[i].state, "RUNNING", sizeof("RUNNING"));
          break;
        case ZOMBIE:
          safestrcpy(uptable[i].state, "ZOMBIE", sizeof("ZOMBIE"));
          break;
        default:
          break;
      }
      i++;
    }
  }
  release(&ptable.lock);
  return i; // return number of uprocs copied into uptable
}
#endif

#ifdef CS333_P3P4
void
readydump(void)
{
  struct proc *p;
  cprintf("Ready processes:\n");

  for(int i = 0; i <= MAXPRIO; i++)
  {
    cprintf("%d: ", i);
    p = ptable.pLists.ready[i];
    while(p) {
      cprintf("(%d, %d) -> ", p->pid, p->budget);
      p = p->next;
    }
    cprintf("\n");
  }
}

void
freedump(void)
{
  struct proc *p;
  int count = 0;
  acquire(&ptable.lock);
  p = ptable.pLists.free;
  cprintf("Free processes:\n");
  while(p) {
    count++;
    p = p->next;
  }
  release(&ptable.lock);

  cprintf("%d\n", count);
}

void
sleepingdump(void)
{
  struct proc *p;
  p = ptable.pLists.sleep;
  cprintf("Sleeping processes:\n");
  while(p) {
    cprintf("%d -> ", p->pid);
    p = p->next;
  }
  cprintf("\n");
}

void
zombiedump(void)
{
  struct proc *p;
  p = ptable.pLists.zombie;
  cprintf("Zombie processes:\n");
  while(p) {
    cprintf("(%d, %d) -> ", p->pid, p->parent->pid);
    p = p->next;
  }
  cprintf("\n");
}
#endif
