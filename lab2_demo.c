//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"
#include "param.h"

#define N 10

int isprime(int n) {
  int i;
  for (i = 2; i < n; ++i) {
    if (n % 2 == 0)
      return 0;
  }
  return 1;
}

void worker(int p) {
  setpriority(p);
  int i;
  for (i = 0; i < 10000; ++i) {
    isprime(i);
  }
  printf(1, "child# %d with priority %d has finished! \n", getpid(), p);
//  sleep(10);
  exit(0);
}

//void worker(int p) {
//  setpriority(p);
//  int j, k;
//  for (j = 0; j < 5000; j++) {
//    for (k = 0; k < 10000; k++) {
//      asm("nop");
//    }
//  }
//  printf(1, "\n child# %d with priority %d has finished! \n", getpid(), p);
//  exit(0);
//}

int main(void) {
  setpriority(0);
  procdump();
  int i;
  for (i = 0; i < N; ++i) {
    int pid = fork();
    if (pid != 0) {
      continue;
    } else {
//      worker((N - i) % MAXPRIORITY);
      worker(50);
    }
  }
  printf(1, "--------------------       after fork all        --------------------\n");
  procdump();
  printf(1, "-------------------- now all my children are zombie --------------------\n");
  sleep(400);
  procdump();
  printf(1, "--------------------   begin waiting for child   --------------------\n");
  while (wait(0) != -1);
  printf(1, "--------------------     cleaned all child       --------------------\n");
  procdump();
  exit(0);
}