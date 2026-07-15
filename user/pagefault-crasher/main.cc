#include "lib/Syscall.h"

extern void main();

extern "C" __attribute__((noreturn)) void _start()
{
  main();
  __builtin_unreachable();
}

void main()
{
  volatile int *p = nullptr;
  *p = 42; // nullptr deref.

  sys_exit();
}
