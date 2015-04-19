#include <stdlib.h>

int64_t atol(const char *str) {
  return strtoll(str, nullptr, 10);
}
