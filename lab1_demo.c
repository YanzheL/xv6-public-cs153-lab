// Test that waitpid() works properly.

#include "types.h"
#include "user.h"

#define N  20

void
worker(int n)
{
  int my_pid = getpid();
  printf(1, "Test process[%d] begins, n = %d...\n", my_pid, n);
  sleep(100);
  printf(1, "Test process[%d] stopped\n", my_pid);
  exit(n);
}

void
lab1_demo()
{
  int main_pid = getpid();
  printf(1, "Main process[%d] begin...\n", main_pid);
  int n;
  int pids[N];
  for(n = 0; n < N; ++n){
    printf(1, "Process[%d] begin fork loop...\n", getpid());
    int pid = fork();
    if(pid == 0){
      worker(n);
    } else{
      pids[n] = pid;
    }
  }

  //TODO: Why the following codes will execute only once??
  //TODO: I forked before, so the following code should run on each process independently
  //TODO: Maybe I misunderstood the fork() mechanism...

  printf(1, "[%d] left loop\n", getpid());
  int pid;
  for(n = 0; n < N; ++n){
    pid = pids[n];
    int status;
    printf(1, "[%d] is waiting for [%d]...\n", main_pid, pid);
//      wait(0);
//      printf(1, "Waited");
    int wtpid = waitpid(pid, &status, 0);
    printf(1, "Waited, target pid = %d, return pid = %d, exit status = %d\n", pid, wtpid, status);
  }
}

int
main(void)
{
  lab1_demo();
  exit(0);
}
