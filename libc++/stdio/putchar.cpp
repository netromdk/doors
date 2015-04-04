#include <stdio.h>

#ifdef __IS_DOORS_KERNEL
  #include <kernel/tty.h>
#endif

int putchar(int ic) {
#ifdef __IS_DOORS_KERNEL
  Tty::putc((char) ic);
#else
  // Not implemented yet..
#endif
  return ic;
}
