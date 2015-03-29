#include <stdio.h>
#include <string.h>

#ifdef __IS_BUROS_KERNEL
  #include <kernel/tty.h>
#endif

int puts(const char *str) {
#ifdef __IS_BUROS_KERNEL
  return Tty::puts(str);
#else
  return printf("%s\n", str);
#endif
}
