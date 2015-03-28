#include <string.h>
#include <stdint.h>

char *strcpy(char *dst, const char *src) {
  return (char*) memcpy(dst, src, strlen(src));
}
