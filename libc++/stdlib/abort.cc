#include <cstdio>
#include <cstdlib>

#ifndef __IS_DOORS_KERNEL
extern "C" void _Exit(int status) __attribute__((__noreturn__));
#endif

__attribute__((__noreturn__)) void abort() noexcept
{
#ifdef __IS_DOORS_KERNEL
  // TODO: Do an actual kernel panic here?
  printf("Kernel Panic: abort()\n");
  while (true) {
  }
#else
  printf("Program aborted!\n");
  _Exit(EXIT_FAILURE);
#endif
}
