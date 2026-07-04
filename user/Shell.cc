#include "lib/Syscall.h"

extern void main();

// ELF entry point.
extern "C" __attribute__((noreturn)) void _start()
{
  main();
  sys_exit();
}

void main()
{
  const char msg[] = "Hello from userland!\n";
  for (const char *p = msg; *p; ++p) {
    sys_write(*p);
  }
}
