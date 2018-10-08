// Test that waitpid() works properly.

#include "types.h"
#include "user.h"

#define N  20

void
lab1_demo()
{
  int parent_pid = getpid();
  printf(1, "Main parent pid = %d\n", parent_pid);
  int n;
  for(n = 0; n < N; ++n){
    int pid = fork();
    if(pid == 0){
      int my_pid = getpid();
      printf(1, "Current test process[%d]\n", my_pid);
      sleep(10);
      exit(n);
    } else{
      int status;
      printf(1, "Process[%d] is waiting for process[%d]\n", parent_pid, pid);
      int wtpid = waitpid(pid, &status, 0);
      printf(1, "Wait finished, target pid = %d, wtpid = %d, exit status = %d\n", pid, wtpid, status);
    }
  }
}

int
main(void)
{
  lab1_demo();
  exit(0);
}
