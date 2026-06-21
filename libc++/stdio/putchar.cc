#include <cstdio>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Tty.h>
#endif

int putchar(int ic)
{
#ifdef __IS_DOORS_KERNEL
  Tty::putc((char) ic);
#else
  // TODO: Not implemented yet..
#endif
  return ic;
}
