//
// Created by Trinity on 19/10/2018.
//

#include "types.h"
#include "user.h"
#include "param.h"

#define N 256

int main(void)
{
  chg_priority(30);
  procdump();
  int i;
  for (i = 0; i < N; ++i) {
    int pid = fork();
    if(pid!=0){

    }
    else{
//      chg_priority(100);
      sleep(200);
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