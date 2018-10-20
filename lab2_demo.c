//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"

#define N 5

int main(void)
{
  chg_priority(50);
  procdump();
  int i;
  for (i = 0; i < N; ++i) {
    int pid = fork();
    if(pid!=0){

    }
    else{
      chg_priority(100);
      sleep(500);
      exit();
    }
  }
  printf(1,"------------------------------\n");
  procdump();
  while (wait()!=-1);
  printf(1,"------------------------------\n");
  procdump();
  exit();
}