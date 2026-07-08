#include "lib/Syscall.h"

extern void main();

extern "C" __attribute__((noreturn)) void _start()
{
  main();
  __builtin_unreachable();
}

void main()
{
  sys_exit();
}
