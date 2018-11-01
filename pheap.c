//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "pheap.h"

//#define PTR_CAST(ptr, size, retptr) unsigned char (*retptr)[size] = ptr;


#define SWAP(pa, pb, T)                \
{                                      \
  T tp = *(pa);                        \
  *(pa) = *(pb);                       \
  *(pb) = tp;                          \
}


int parent(int i) { return (i - 1) / 2; }

int left(int i) { return (2 * i + 1); }

int right(int i) { return (2 * i + 2); }

void hpush(int idx, int key, struct pheap *h) {
  if (h->size == NPROC)
    return;
  ++h->size;
  int i = h->size - 1;
  h->nodes[i].idx = idx;
  h->nodes[i].key = key;
  while (i != 0 && h->nodes[parent(i)].key < h->nodes[i].key) {
    SWAP(h->nodes + i, h->nodes + parent(i),struct hitem);
    i = parent(i);
  }
}

void MaxHeapify(int idx, struct pheap *h) {
  int l = left(idx), r = right(idx), max_idx = idx;
  if (l < h->size && h->nodes[l].key > h->nodes[idx].key)
    max_idx = l;
  if (r < h->size && h->nodes[r].key > h->nodes[max_idx].key)
    max_idx = r;
  if (max_idx != idx) {
    SWAP(h->nodes + idx, h->nodes + max_idx, struct hitem);
    MaxHeapify(max_idx, h);
  }
}

// Return max priority proc index in ptable
int
hpop(struct pheap *h) {
  if (h->size <= 0)
    return -1;
  if (h->size == 1) {
    --h->size;
    return h->nodes[0].idx;
  }

  // Store the max value, and remove it from heap
  int root = h->nodes[0].idx;
  h->nodes[0] = h->nodes[h->size - 1];
  --h->size;
  MaxHeapify(0, h);
  return root;
}
