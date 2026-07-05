#include <cstdio>
#include <cstdlib>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Panic.h>
#elif !defined(__IS_DOORS_USERLAND)
extern "C" void _Exit(int status) __attribute__((__noreturn__));
#endif

__attribute__((__noreturn__)) void abort() noexcept
{
#ifdef __IS_DOORS_KERNEL
  panic("abort()");
#elif defined(__IS_DOORS_USERLAND)
  constexpr unsigned int SYS_PANIC = 10;
  __asm__ volatile("int $0x80" : : "a"(SYS_PANIC), "b"(0U) : "memory");
  __builtin_unreachable();
#else
  printf("Program aborted!\n");
  _Exit(EXIT_FAILURE);
#endif
}
