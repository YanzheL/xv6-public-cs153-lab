#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

//struct queue {
//    struct spinlock lock;
////    struct proc *proc[NPROC];
////    int size;
//} readyq;

struct hitem {
  int *key;
  int idx;
};

struct pheap {
  struct spinlock lock;
  struct hitem nodes[NPROC + 1];
  int size; // Current number of elements in max heap
} readyq;

//#define TCCR    (0x0390/4)   // Timer Current Count
//#define GETPROCKEY(p) p->vruntime - readyq.min
//#define GETPROCKEY(p) p->vruntime

uint clock() {
//  return ticks * 100 + (10000000 - lapic[TCCR]) / 100000;
//  return ticks + ((10000000 - lapic[TCCR]) >= 5000000);
  return ticks;
}

//// Used in qsort()
//int partition(struct proc **src, int low, int high) {
//  struct proc *pv = src[low];
//  int pivot = GETPROCKEY(pv);
//  while (low < high) {
//    while (low < high && GETPROCKEY(src[high]) <= pivot)--high;
//    src[low] = src[high];
//    while (low < high && GETPROCKEY(src[low]) >= pivot)++low;
//    src[high] = src[low];
//  }
//  src[low] = pv;
//  return low;
//}
//
//// Quick sort over ready queue
//void qsort(struct proc **arr, int low, int high) {
//  if(low < high) {
//    int pi = partition(arr, low, high);
//    qsort(arr, low, pi - 1);
//    qsort(arr, pi + 1, high);
//  }
//}
//
//void sort(struct proc *arr[], int size) {
//  int i, j;
//  int p1, p2;
//  for (i = 0; i < size; ++i) {
//    for (j = 0; j < size - 1; ++j) {
//      p1 = GETPROCKEY(arr[j]);
//      p2 = GETPROCKEY(arr[j + 1]);
//      if(p1 < p2) SWAP(&arr[j], &arr[j + 1], struct proc*)
//    }
//  }
//}


static struct proc *initproc;

int nextpid = 1;

extern void forkret(void);

extern void trapret(void);

static void wakeup1(void *chan);

void
change_state(struct proc *p, enum procstate s2) {
//  static char *states[] = {
//      [UNUSED]    "UNUSED  ",
//      [EMBRYO]    "EMBRYO  ",
//      [SLEEPING]  "SLEEPING",
//      [RUNNABLE]  "RUNNABLE",
//      [RUNNING]   "RUNNING ",
//      [ZOMBIE]    "ZOMBIE  "
//  };
//  cprintf("[%s] -> [%s]\n",states[p->state],states[s2]);
  if(!holding(&ptable.lock))
    panic("Unprotected state change");
  p->delta_exec_weighted = 1 + p->priority - p->donations.total;
  uint curtime = clock();
  switch (p->state) {
    case UNUSED:    // can only goto EMBRYO
      break;
    case EMBRYO:    // can only goto RUNNABLE
      p->tmstat.birthticks = curtime;
      p->tmstat.lastrun = curtime;
      p->tmstat.beginpending = curtime;
      break;
    case RUNNABLE:  // can only goto RUNNING
      p->tmstat.beginrun = curtime;
      p->tmstat.pendingticks += curtime - p->tmstat.beginpending;
      break;
    case RUNNING:
      p->tmstat.lastrun = curtime;
      p->tmstat.runticks += curtime - p->tmstat.beginrun;
      p->vruntime += (curtime - p->tmstat.beginrun) * p->delta_exec_weighted;
      switch (s2) {
        case SLEEPING:
          p->tmstat.beginsleep = curtime;
          break;
        case RUNNABLE:
          p->tmstat.beginpending = curtime;
          break;
        case ZOMBIE:
          p->tmstat.dieticks = curtime;
          break;
        default:
          // Nothing should reach here
          //  cprintf("11111[%s] -> [%s]\n",states[p->state],states[s2]);
          break;
      }
      break;
    case SLEEPING:
      p->tmstat.beginpending = curtime;
      p->tmstat.sleepticks += curtime - p->tmstat.beginsleep;
      break;
    case ZOMBIE:
      memset(&p->tmstat, 0, sizeof(p->tmstat));
      break;
    default:
      // Nothing should reach here
      // cprintf("22222[%s] -> [%s]\n",states[p->state],states[s2]);
      break;
  }

  p->state = s2;
}

static inline int parent(int i) { return (i - 1) / 2; }

static inline int left(int i) { return (2 * i + 1); }

static inline int right(int i) { return (2 * i + 2); }

void MinHeapify(int root, struct pheap *h) {
  int l = left(root), r = right(root), min_idx = root;
  if(l < h->size && *(h->nodes[l].key) < *(h->nodes[root].key))
    min_idx = l;
  if(r < h->size && *(h->nodes[r].key) < *(h->nodes[min_idx].key))
    min_idx = r;
  if(min_idx != root) {
    SWAP(h->nodes + root, h->nodes + min_idx, struct hitem);
    MinHeapify(min_idx, h);
  }
}

void hpush(int idx, int *key, struct pheap *h) {
  acquire(&h->lock);
  if(h->size == NPROC)
    goto done;
  ++h->size;
  int i = h->size - 1;
  h->nodes[i].idx = idx;
  h->nodes[i].key = key;
  while (i != 0 && *(h->nodes[parent(i)].key) > *(h->nodes[i].key)) {
    SWAP(h->nodes + i, h->nodes + parent(i), struct hitem);
    i = parent(i);
  }
  done:
  release(&h->lock);
}

// Return min priority proc index in ptable
int
hpop(struct pheap *h) {
  int root = -1;
  acquire(&h->lock);
  if(h->size <= 0)
    goto done;
  if(h->size == 1) {
    --h->size;
    root = h->nodes[0].idx;
    goto done;
  }
  // Store the min value, and remove it from heap
  root = h->nodes[0].idx;
  h->nodes[0] = h->nodes[h->size - 1];
  --h->size;
  MinHeapify(0, h);
  done:
  release(&h->lock);
  return root;
}


void
readyq_push(struct proc *p) {
  if(!holding(&ptable.lock))
    panic("Unprotected readyq_push()");
  change_state(p, RUNNABLE);
  hpush(p->pidx, &p->vruntime, &readyq);
}

struct proc *
readyq_pop() {
  struct proc *p = 0;
  int idx = hpop(&readyq);
  if(idx != -1)
    p = ptable.proc + idx;
  return p;
}

void
pinit(void) {
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu *
mycpu(void) {
  int apicid, i;

  if(readeflags() & FL_IF)
    panic("mycpu called with interrupts enabled\n");

  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if(cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc *
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc *
allocproc(void) {
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

  found:
//  p->state = EMBRYO;
  change_state(p, EMBRYO);
  p->pid = nextpid++;
  p->pidx = (int) (p - ptable.proc);
  p->priority = 20;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0) {
//    p->state = UNUSED;
    change_state(p, UNUSED);
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe *) sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint *) sp = (uint) trapret;

  sp -= sizeof *p->context;
  p->context = (struct context *) sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint) forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void) {
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();

  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int) _binary_initcode_size);
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

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  readyq_push(p);
//  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n) {
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0) {
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0) {
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void) {
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0) {
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
    kfree(np->kstack);
    np->kstack = 0;
//    np->state = UNUSED;
    change_state(np, UNUSED);
    return -1;
  }
  np->priority = curproc->priority;
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for (i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  readyq_push(np);
//  np->state = RUNNABLE;
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(int status) {
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for (fd = 0; fd < NOFILE; fd++) {
    if(curproc->ofile[fd]) {
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  curproc->exitstatus = status;

  // Pass abandoned children to init.
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->parent == curproc) {
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  change_state(curproc, ZOMBIE);
  sched();
  panic("zombie exit");
}

int
donate(int pid) {
  struct proc *curproc = myproc();
  struct proc *p;
  int i, j;
  int ret = -1;
  struct donator *d;
  acquire(&ptable.lock);
  for (i = 0; i < NPROC; ++i) {
    p = ptable.proc + i;
    if(p->pid != pid)
      continue;
    for (j = 0; j < MAXDONATION; ++j) {
      d = p->donations.donators + j;
      if(d->p == 0) {
        d->p = curproc;
        d->priority = curproc->priority;
        p->donations.total += curproc->priority;
        ret = p->priority + p->donations.total;
        goto done;
      }
    }
  }
  done:
  release(&ptable.lock);
  yield();
  return ret;
}

int
undonate(int pid) {
  struct proc *curproc = myproc();
  struct proc *p;
  int i, j;
  int ret = -1;
  struct donator *d;
  acquire(&ptable.lock);
  for (i = 0; i < NPROC; ++i) {
    p = ptable.proc + i;
    if(p->pid != pid)
      continue;
    for (j = 0; j < MAXDONATION; ++j) {
      d = p->donations.donators + j;
      if(d->p != 0 && d->p->pid == curproc->pid) {
        d->p = 0;
        p->donations.total -= d->priority;
        d->priority = 0;
        ret = p->priority + p->donations.total;
        goto done;
      }
    }
  }
  done:
  release(&ptable.lock);
  yield();
  return ret;
}

int
setpriority(int priority) {
  if(priority >= MAXPRIORITY)
    return -1;
  acquire(&ptable.lock);
  struct proc *curproc = myproc();
  int prev = curproc->priority;
  curproc->priority = priority;
  release(&ptable.lock);
  yield();
  return prev;
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(int *status) {
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();

  acquire(&ptable.lock);
  for (;;) {
    // Scan through table looking for exited children.
    havekids = 0;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE) {
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->priority = 20;
        p->killed = 0;
        p->vruntime = 0;
        p->delta_exec_weighted = 0;
        change_state(p, UNUSED);
//        p->state = UNUSED;
        if(status)
          *status = p->exitstatus;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed) {
      if(status)
        *status = -1;
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//int
//donate(int pid, int priority){
//  struct proc *p;
//  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
//    if (p->pid != pid)
//      continue;
//
//  }
//}

int
waitpid(int pid, int *status, int options) {
  struct proc *p;
  int found;
  struct proc *curproc = myproc();

  acquire(&ptable.lock);
  for (;;) {
    // Scan through table looking for exited proc with pid.
    found = 0;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->pid != pid)
        continue;
      found = 1;
      if(p->state == ZOMBIE) {
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        change_state(p, UNUSED);
        if(status)
          *status = p->exitstatus;
        release(&ptable.lock);
        return pid;
      }
    }
    if(!found || curproc->killed) {
      if(status)
        *status = -1;
      release(&ptable.lock);
      return -1;
    }
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.

void
scheduler(void) {
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for (;;) {
    // Enable interrupts on this processor.
    sti();
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    while (p = readyq_pop()) {
//      acquire(&q->lock);
//      p = q->proc[--q->size];
//      release(&q->lock);
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      change_state(p, RUNNING);
      swtch(&(c->scheduler), p->context);
      switchkvm();
      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;

    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void) {
  int intena;
  struct proc *p = myproc();
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags() & FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void) {
  acquire(&ptable.lock);  //DOC: yieldlock
  readyq_push(myproc());
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void) {
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if(first) {
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
void
sleep(void *chan, struct spinlock *lk) {
  struct proc *p = myproc();

  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock) {  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  change_state(p, SLEEPING);
  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock) {  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan) {
  struct proc *p;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      readyq_push(p);
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan) {
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid) {
  struct proc *p;

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->pid == pid) {
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        readyq_push(p);
//        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.

void
procdump(void) {
  procinfo(-1);
}

int
procinfo(int pid) {
  struct proc *p;
  int i, found = 0;
  if(pid == -1)
    p = myproc();
  else {
    acquire(&ptable.lock);
    for (i = 0; i < NPROC; ++i) {
      p = ptable.proc + i;
      if(p->pid == pid) {
        found = 1;
        break;
      }
    }
    release(&ptable.lock);
    if(!found)
      return -1;
  }


  static char *states[] = {
      [UNUSED]    "unused",
      [EMBRYO]    "embryo",
      [SLEEPING]  "sleep ",
      [RUNNABLE]  "runble",
      [RUNNING]   "run   ",
      [ZOMBIE]    "zombie"
  };

  char *state;
  if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
    state = states[p->state];
  else
    state = "???\n";

  uint turnaround = p->tmstat.lastrun - p->tmstat.birthticks + 1;
  cprintf("pid=%d\t"
          "state=%s\t"
          "name=%s\t"
          "idx=%d\t"
          "priority=%d\t"
          "donations=%d\t"
          "vruntime=%d\t"
          "runticks=%u\t"
          "sleepticks=%u\t"
          "pendingticks=%u  \t"
          "turnaround=%u\n"
//          "err=%d\n"
//          "beginrun=%d\t"
//          "lastrun=%u\t"
//          "beginpending=%u\n"
      ,
          p->pid,
          state,
          p->name,
          p->pidx,
          p->priority,
          p->donations.total,
          p->vruntime,
          p->tmstat.runticks,
          p->tmstat.sleepticks,
          p->tmstat.pendingticks,
          turnaround
//          turnaround - p->tmstat.sleepticks - p->tmstat.pendingticks - p->tmstat.runticks
//          p->tmstat.beginrun,
//          p->tmstat.lastrun,
//          p->tmstat.beginpending
  );
  return pid;
}

//void
//procinfo(void)
//{
//  static char *states[] = {
//  [UNUSED]    "unused",
//  [EMBRYO]    "embryo",
//  [SLEEPING]  "sleep ",
//  [RUNNABLE]  "runble",
//  [RUNNING]   "run   ",
//  [ZOMBIE]    "zombie"
//  };
////  int i;
//  struct proc *p;
//  char *state;
////  uint pc[10];
//
//  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//    if(p->state == UNUSED)
//      continue;
//    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
//      state = states[p->state];
//    else
//      state = "???\n";
//
////    struct tmspec turnaround;
////    clocksub(p->tmstat.dieticks,p->tmstat.birthticks,&turnaround);
//    uint turnaround = p->tmstat.lastrun-p->tmstat.birthticks;
//
////    cprintf("pid=%d\t"
////            "state=%s\t"
////            "name=%s\t"
////            "idx=%d\t"
////            "priority=%d\t"
////            "runticks=%u.%u\t"
////            "sleepticks=%u.%u\t"
////            "pendingticks=%u.%u\t"
////            "turnaround=%u.%u\t"
////            "lastrun=%u.%u\t"
////            "TCCR=%u\n",
////            p->pid,
////            state,
////            p->name,
////            p->pidx,
////            p->priority,
////            p->tmstat.runticks.ticks,
////            p->tmstat.runticks.nano,
////            p->tmstat.sleepticks.ticks,
////            p->tmstat.sleepticks.nano,
////            p->tmstat.pendingticks.ticks,
////            p->tmstat.pendingticks.nano,
////            turnaround.ticks,
////            turnaround.nano,
////            p->tmstat.lastrun.ticks,
////            p->tmstat.lastrun.nano,
////            lapic[TCCR]
////            );
//    cprintf("pid=%d\t"
//            "state=%s\t"
//            "name=%s\t"
//            "idx=%d\t"
//            "priority=%d\t"
//            "runticks=%u\t"
//            "sleepticks=%u\t"
//            "pendingticks=%u\t"
//            "turnaround=%u\t"
//            "birthticks=%u\t"
//            "lastrun=%u\n",
//            p->pid,
//            state,
//            p->name,
//            p->pidx,
//            p->priority,
//            p->tmstat.runticks,
//            p->tmstat.sleepticks,
//            p->tmstat.pendingticks,
//            turnaround,
//            p->tmstat.birthticks,
//            p->tmstat.lastrun
//    );
////    if(p->state == SLEEPING){
////      getcallerpcs((uint*)p->context->ebp+2, pc);
////      cprintf("pc=");
////      for(i=0; i<10 && pc[i] != 0; i++)
////        cprintf("%p, ", pc[i]);
////    }
////    cprintf("\n");
//  }
//}
