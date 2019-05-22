/********** test.c file *************/
//Replace this code and rename to init.c to run my own operating system.
#include "ucode.c"

int main(int argc, char *argv[ ])
{
  int i;

  int pid = getpid();
  printf("KCW: PROC %d running test program\n", pid);

  printf("argc = %d\n", argc);
  for (i=0; i<argc; i++)
    printf("argv[%d] = %s\n", i, argv[i]);

  printf("PROC %d exit\n", pid);
}

