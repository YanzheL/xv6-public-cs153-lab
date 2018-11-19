//
// Created by Yanzhe Li on 2018-11-18.
//

#include "types.h"
#include "user.h"

#define N 1234

int main() {
  int s[N];
  int i, j;
  sbrk(1998);
  procinfo(getpid());

  printf(1, "&i=%x, &j=%x, s=%x\n", &i, &j, s);
  for (i = 0; i < N; ++i) {
    s[i] = 1;
  }

  printf(1, "&s[begin]=%x, value=%d\n", s, s[0]);
  printf(1, "&s[end]=%x, value=%d\n", &s[N - 1], s[N - 1]);
  memdump();
  exit();
}