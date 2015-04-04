#include <string.h>
#include <algorithm.h>

int strncmp(const char *str1, const char *str2, size_t num) {
  size_t len1 = strlen(str1), len2 = strlen(str2);
  size_t m = min(len1, len2);
  return memcmp(str1, str2, min(m, num));
}
