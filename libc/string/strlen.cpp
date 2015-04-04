#include <string.h>

size_t strlen(const char *str) {
  auto *pos = str;
  for (; *pos; ++pos);
  return pos - str;
}
