#include <string.h>

/**
 * If str is shorter than the number of chars to copy then pad with
 * zeroes by the difference.
 */
char *strncpy(char *dst, const char *src, size_t num) {
  size_t len = strlen(src);
  if (len < num) {
    memcpy(dst, src, len);
    memset(dst+len, 0, num-len);
  }
  else {
    memcpy(dst, src, num);
  }
  return dst;
}
