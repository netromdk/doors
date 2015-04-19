#include <stdlib.h>

uint32_t strtoul(const char *str, char **endptr, int base) {
  return strtoull(str, endptr, base);
}
