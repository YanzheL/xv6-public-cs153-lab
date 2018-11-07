//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"
#include "param.h"

#define N 60

//int isprime(int n) {
//  int i;
//  for (i = 2; i < n; ++i) {
//    if (n % 2 == 0)
//      return 0;
//  }
//  return 1;
//}
//
//void worker(int p) {
//  setpriority(p);
//  int i,ct=0;
//  for (i = 0; i < 10000; ++i) {
//    if(isprime(i))
//      ++ct;
//  }
//  printf(1, "child# %d with priority %d has finished! \n", getpid(), p);
////  sleep(10);
//  exit(ct);
//}

//int stopped[N];

void worker(int p) {
  setpriority(p);
  int j, k;
  for (j = 0; j < 5000; j++) {
    for (k = 0; k < 10000; k++) {
      asm("nop");
    }
  }
  sleep(10);
//  printf(1, "\tchild# %d with priority %d has finished!\n", getpid(), p);
//  stopped[id] = 1;
  procdump();
  exit(0);
}

int main(void) {
  setpriority(0);
  procdump();
  int i;
  for (i = 0; i < N; ++i) {
    int pr = (MAXPRIORITY - i - 1);
    pr = pr >= 0 ? pr : -pr;
    if (pr == 0)
      pr = 1;
    pr %= MAXPRIORITY;
    int pid = fork();
    if (pid != 0) {
      continue;
    } else {
//      worker((N - i) % MAXPRIORITY);
      worker(pr);
//      worker(50);
    }
  }

  printf(1, "--------------------   begin waiting for child   --------------------\n");
  while (wait(0) != -1);
  wait(0);
  procdump();
  printf(1, "--------------------     cleaned all child       --------------------\n");
  exit(0);
}