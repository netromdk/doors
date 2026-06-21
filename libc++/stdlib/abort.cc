#include <cstdio>
#include <cstdlib>

#ifndef __IS_DOORS_KERNEL
extern "C" void _Exit(int status) __attribute__((__noreturn__));
#else
#include <kernel/Panic.h>
#endif

__attribute__((__noreturn__)) void abort() noexcept
{
#ifdef __IS_DOORS_KERNEL
  panic("abort()");
#else
  printf("Program aborted!\n");
  _Exit(EXIT_FAILURE);
#endif
}
