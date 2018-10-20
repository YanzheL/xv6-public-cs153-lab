//
// Created by Trinity on 19/10/2018.
//

struct hitem {
    int key;
    int idx;
};

struct pheap {
    struct hitem nodes[NPROC];
    int size; // Current number of elements in max heap
};