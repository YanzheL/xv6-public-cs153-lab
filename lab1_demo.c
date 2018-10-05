// Test that waitpid() works properly.

#include "types.h"
#include "user.h"

#define N  20

void
lab1_demo(void)
{
  int pid = fork();
  int status;
  if (pid == 0) {
    printf(1, "Current test process pid = %d\n", getpid());
    sleep(10);
    exit(5);
  } else {
    printf(1, "Waiting for pid = %d\n", pid);
    int wtpid = waitpid(pid, &status, 0);
    printf(1, "Wait finish, pid = %d, wtpid = %d, exit status = %d\n", pid, wtpid, status);
  }
}

int
main(void)
{
  int n;
  for (n = 0; n < N; ++n) {
    lab1_demo();
  }
  exit(0);
}
