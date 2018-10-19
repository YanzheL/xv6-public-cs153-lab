//
// Created by Trinity on 19/10/2018.
//

#ifndef XV6_PUBLIC_CS153_LAB_PHEAP_H
#define XV6_PUBLIC_CS153_LAB_PHEAP_H

#include "param.h"
#include "defs.h"
#include "proc.h"

struct pheap_s {
    struct ptable_s *table;
    int indexes[NPROC]; // pointer to array of elements in heap
    int size; // Current number of elements in min heap
};

inline int parent(int i) { return (i - 1) / 2; }

// to get index of left child of node at index i
inline int left(int i) { return (2 * i + 1); }

// to get index of right child of node at index i
inline int right(int i) { return (2 * i + 2); }

// Returns the minimum key (key at root) from min heap
inline int getMin(struct pheap_s *h) { return h->indexes[0]; }

int extractMin(struct pheap_s *h);

void insertKey(struct proc *p, struct pheap_s *h);


#endif //XV6_PUBLIC_CS153_LAB_PHEAP_H
