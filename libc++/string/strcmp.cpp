#include <string.h>
#include <algorithm.h>

int strcmp(const char *str1, const char *str2) {
  size_t len1 = strlen(str1), len2 = strlen(str2);
  return memcmp(str1, str2, min(len1, len2));
}
