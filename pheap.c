//
// Created by Trinity on 19/10/2018.
//

#include "pheap_s.h"


#define PTR_CAST(ptr, size, retptr)    \
unsigned char (*retptr)[size] = ptr;

#define SWAP(pa, pb, T)                \
{                                      \
  T tp = *(pa);                        \
  *(pa) = *(pb);                       \
  *(pb) = tp;                          \
}


// Inserts a new key 'k'
void insertKey(struct proc *p, struct pheap_s *h) {
  if (h->size == NPROC)
    panic("Overflow: Could not insertKey");
  ++h->size;
  int i = h->size - 1;
  h->indexes[i] = p->pidx;
  struct proc *procs = h->table->proc;
  while (i != 0 && procs[h->indexes[parent(i)]].priority < procs[h->indexes[i]].priority) {
    SWAP(h->indexes + i, h->indexes + parent(i), int)
    i = parent(i);
  }
}

void MaxHeapify(int idx, struct pheap_s *h) {
  int l = left(idx);
  int r = right(idx);
  int max_idx = idx;
  struct proc *procs = h->table->proc;
  if (l < h->size && procs[h->indexes[l]].priority > procs[h->indexes[idx]].priority)
    max_idx = l;
  if (r < h->size && procs[h->indexes[r]].priority > procs[h->indexes[max_idx]].priority)
    max_idx = r;
  if (max_idx != idx) {
    SWAP(h->indexes + idx, h->indexes + max_idx, int);
    MaxHeapify(max_idx, h);
  }
}

// Return max priority proc index in ptable
int
extractMin(struct pheap_s *h) {
  if (h->size <= 0)
    return 0;
  if (h->size == 1) {
    --h->size;
    return h->indexes[0];
  }

  // Store the minimum value, and remove it from heap
  int root = h->indexes[0];
  h->indexes[0] = h->indexes[h->size - 1];
  --h->size;
  MaxHeapify(0, h);
  return root;
}

//// Inserts a new key 'k'
//void insertKey(int k, struct pheap_s *h, void* table, int psize) {
//  if (h->size == NPROC)
//    panic("Overflow: Could not insertKey");
//
//  PTR_CAST(table,psize,rtable);
//
//  // First insert the new key at the end
//  ++h->size;
//  int i = h->size - 1;
//  h->indexes[i] = k;
//
//  // Fix the min heap property if it is violated
//  while (i != 0 && rtable[h->indexes[parent(i)]] < rtable[h->indexes[i]]) {
//    SWAP(h->indexes+i, h->indexes+parent(i),int)
//    i = parent(i);
//  }
//}

