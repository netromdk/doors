#include <stdio.h>
#include <string.h>

#ifdef __IS_DOORS_KERNEL
  #include <kernel/tty.h>
#endif

int puts(const char *str) {
#ifdef __IS_DOORS_KERNEL
  return Tty::puts(str);
#else
  // TODO: Write data to stdout when defined.
  return strlen(str);
#endif
}
