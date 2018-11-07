//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"
#include "param.h"

#define N 100

//int isprime(int n) {
//  int i;
//  for (i = 2; i < n; ++i) {
//    if (n % 2 == 0)
//      return 0;
//  }
//  return 1;
//}
//
//void b3_worker(int p) {
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

void b2_worker(void) {
  printf(1, "--------------------   child begin   --------------------\n");
  procdump();
  int j, k;
  for (j = 0; j < 5000; j++) {
    for (k = 0; k < 10000; k++) {
      asm("nop");
    }
  }
  printf(1, "--------------------   child end   --------------------\n");
  procdump();
  exit(0);
}


void test_bouns2(void) {
  printf(1, "--------------------  before fork  --------------------\n");
  procdump();
  int j, k;
  int pid = fork();
  if(pid) {
    for (j = 0; j < 1000; j++) {
      for (k = 0; k < 10000; k++) {
        asm("nop");
      }
    }
    donate(pid);
    printf(1, "--------------------   after donate   --------------------\n");
    procinfo(pid);
    sleep(10);
    reset_donate(pid);
    printf(1, "--------------------   after reset    --------------------\n");
    procinfo(pid);
    wait(0);
  } else {
    b2_worker();
  }

}

void b3_worker(int p) {
  setpriority(p);
  int j, k;
  for (j = 0; j < 5000; j++) {
    for (k = 0; k < 10000; k++) {
      asm("nop");
    }
  }
  sleep(10);
  procdump();
  exit(0);
}

void test_bouns3(void) {
  setpriority(0);
  int i, pr;
  for (i = 0; i < N; ++i) {
    pr = (MAXPRIORITY - i * 10);
    pr = pr >= 0 ? pr : -pr;
    pr %= MAXPRIORITY;
    if(pr == 0)
      pr = 1;
    int pid = fork();
    if(pid != 0) {
      continue;
    } else {
      b3_worker(pr);
    }
  }

//  printf(1, "--------------------   begin waiting for child   --------------------\n");
  while (wait(0) != -1);
  procdump();
//  printf(1, "--------------------     cleaned all child       --------------------\n");
//  exit(0);
}

int main(int argc, char *argv[]) {
  if(atoi(argv[1]) == 1)
    test_bouns3();
  else if(atoi(argv[1]) == 2)
    test_bouns2();
  else if(atoi(argv[1]) == 3)
    test_bouns3();
  else
    printf(1,
           "\ntype \"lab2_demo n\" to test bouns n\n");
  exit(0);
}