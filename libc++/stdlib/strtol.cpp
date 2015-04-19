#include <stdlib.h>

int32_t strtol(const char *str, char **endptr, int base) {
  return strtoll(str, endptr, base);
}
