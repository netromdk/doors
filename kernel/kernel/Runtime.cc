#include <kernel/Runtime.h>

int __cxa_guard_acquire(__guard *g)
{
  return !*g;
}

void __cxa_guard_release(__guard *g)
{
  *g = 1;
}

extern "C" int __cxa_atexit(void (*)(void *), void *, void *)
{
  // Kernel never exits, so destructors registered here are never called.
  return 0;
}
