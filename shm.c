#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct
{
  struct spinlock lock;
  struct shm_page
  {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit()
{
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i < 64; i++) {
    shm_table.shm_pages[i].id = 0;
    shm_table.shm_pages[i].frame = 0;
    shm_table.shm_pages[i].refcnt = 0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer)
{
  acquire(&shm_table.lock);
  struct shm_page *p;
  struct shm_page *np = 0;
  struct shm_page *end = shm_table.shm_pages + 64;
  struct proc *curproc = myproc();
  char *mem;

  for (p = shm_table.shm_pages; p < end; ++p) {
    if(p->id == 0)
      np = p; // Find available slot in same loop
    if(p->id == id)
      break;
  }
  if(p == end) { // Not found
    if(np == 0)
      goto bad;
    if((mem = kalloc()) == 0)
      goto bad;
    memset(mem, 0, PGSIZE);
    if(mappages(curproc->pgdir, (void *) curproc->shmtop, PGSIZE, V2P(mem), PTE_U | PTE_W) < 0) {
      kfree(mem);
      goto bad;
    }
    pgref_inc(V2P(mem));
    *pointer = (char *) curproc->shmtop;
    np->id = id;
    np->frame = mem;
    np->refcnt = 1;
    curproc->shmtop += PGSIZE;
  } else { // Found
    mem = p->frame;
    if(mappages(curproc->pgdir, (void *) curproc->shmtop, PGSIZE, V2P(mem), PTE_U | PTE_W) < 0)
      goto bad;
    pgref_inc(V2P(mem));
    *pointer = (char *) curproc->shmtop;
    ++p->refcnt;
    curproc->shmtop += PGSIZE;
  }

  lcr3(V2P(curproc->pgdir));
  release(&shm_table.lock);
  return 0;

  bad:
  lcr3(V2P(curproc->pgdir));
  release(&shm_table.lock);
  return -1;
}

int shm_close(int id)
{
  acquire(&shm_table.lock);
  struct shm_page *p;
  struct shm_page *end = shm_table.shm_pages + 64;

  for (p = shm_table.shm_pages; p < end; ++p) {
    if(p->id == id)
      break;
  }
  if(p == end) // Not found
    goto bad;

  if(p->refcnt <= 1) {
    p->id = 0;
    kfree(p->frame);
    p->frame = 0;
  } else
    --p->refcnt;

  release(&shm_table.lock);
  return 0;

  bad:
  release(&shm_table.lock);
  return -1;
}
