// Test that waitpid() works properly.

#include "types.h"
#include "user.h"

#define N  20

// example child process task
void
worker(int n)
{
  int my_pid = getpid();
  printf(1, "Test process[%d] begins, n = %d...\n", my_pid, n);
  sleep(100);
  printf(1, "Test process[%d] stopped\n", my_pid);
  // use n as exit status code
  exit(n);
}

void
lab1_demo()
{
  int main_pid = getpid();
  printf(1, "Main process[%d] begin...\n", main_pid);
  int n;
  int pids[N];
  //Create N test processes, and store their pid in pids.
  for(n = 0; n < N; ++n){
    printf(1, "Process[%d] begin fork loop...\n", getpid());
    int pid = fork();
    if(pid == 0){
      // use the index of current child process's pid in pids to test whether exit status return properly.
      // we can get their exit status later when calling waitpid().
      worker(n);
    } else{
      pids[n] = pid;
    }
  }
  //After creating these test processes, we begin to wait for each of them by their pid from pids.
  printf(1, "[%d] left loop\n", getpid());
  int pid;
  for(n = 0; n < N; ++n){
    pid = pids[n];
    int status;
    printf(1, "[%d] is waiting for [%d]...\n", main_pid, pid);
    int wtpid = waitpid(pid, &status, 0);
    // if waitpid() works,
    // target pid should always equal to return pid.
    // exit status should always equal to current index n.
    printf(1, "Waited, target pid = %d, return pid = %d, exit status = %d, n = %d\n", pid, wtpid, status, n);
  }
}

int
main(void)
{
  lab1_demo();
  exit(0);
}
