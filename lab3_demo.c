//
// Created by Yanzhe Li on 2018-11-18.
//

#include "types.h"
#include "user.h"

int test1()
{
  const int n = 123456;
  const int h_size = 4321;
  int s[n];
  int i;
  sbrk(h_size);
  printf(1, "Created int array of size %d, and allocated %d bytes on heap.\n", n, h_size);
  procinfo(getpid());

  for (i = 0; i < n; ++i)
    s[i] = 1;

  printf(1, "&s[begin]=0x%x\tvalue=%d\n", s, s[0]);
  printf(1, "&s[end]=0x%x\tvalue=%d\n", &s[n - 1], s[n - 1]);
  return 0;
}

int test2()
{
  uint *bad_pointer = (uint *) 0x7fff0000;
  printf(1, "Trying to write to illegal address at 0x%p, this should throw a exception.\n", bad_pointer);
  *bad_pointer = 0xFFFFFFFF;
  procinfo(getpid());
  return 0;
}

int test3()
{
  const int n = 1970;
  const int h_size = 1120;
  int s[n];
  int i;
  sbrk(h_size);
  printf(1, "Created int array of size %d, and allocated %d bytes on heap.\n", n, h_size);
  procinfo(getpid());

  for (i = 0; i < n; ++i)
    s[i] = 1;

  printf(1, "&s[begin]=0x%x\tvalue=%d\n", s, s[0]);
  printf(1, "&s[end]=0x%x\tvalue=%d\n", &s[n - 1], s[n - 1]);
  memdump();
  return 0;
}

int main(int argc, char *argv[])
{
  if(atoi(argv[1]) == 1)
    test1();
  else if(atoi(argv[1]) == 2)
    test2();
  else if(atoi(argv[1]) == 3)
    test3();
  else
    printf(1,
           "\ntype \"lab3_demo n\" to test n\n"
           "test 1: large stack allocation\n"
           "test 2: bad_pointer detection, should throw EXC_BAD_ACCESS\n"
           "test 3: stack allocation with memory dump\n"
    );
  exit();
}