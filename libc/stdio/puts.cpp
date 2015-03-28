#include <stdio.h>
#include <string.h>

#ifdef __IS_BUROS_KERNEL
  #include <kernel/tty.h>
#endif

int puts(const char *str) {
#ifdef __IS_BUROS_KERNEL
  termPuts(str);
  return strlen(str);
#else
  return printf("%s\n", str);
#endif
}
