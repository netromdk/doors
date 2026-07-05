#include <cstdio>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Tty.h>
#endif

int putchar(int ic)
{
#ifdef __IS_DOORS_KERNEL
  Tty::putc((char) ic);
#elif defined(__IS_DOORS_USERLAND)
  constexpr unsigned int SYS_WRITE = 1;
  __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"(static_cast<unsigned int>(ic)) : "memory");
#else
  // TODO: Not implemented yet..
#endif
  return ic;
}
