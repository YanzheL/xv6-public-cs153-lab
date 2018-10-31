//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"
#include "param.h"

#define N 16

int isprime(int n){
  int i;
  for (i = 2; i < n; ++i) {
    if(n%2==0)
      return 0;
  }
  return 1;
}

void worker(int p)
{
  setpriority(p);
  int i;
  for (i = 0; i < 10000; ++i) {
    isprime(i);
  }
  sleep(50);
  exit(0);
}

int main(void)
{
  setpriority(15);
  procdump();
  int i;
  for (i = 0; i < N; ++i) {
    int pid = fork();
    if(pid!=0){

    }
    else{
      worker(i);
    }
  }
  printf(1,"--------------------       after fork all        --------------------\n");
  procdump();
  printf(1, "-------------------- now all my children are zombie --------------------\n");
  sleep(400);
  procdump();
  printf(1,"--------------------   begin waiting for child   --------------------\n");
  while (wait(0) != -1);
  printf(1,"--------------------     cleaned all child       --------------------\n");
  procdump();
  exit(0);
}