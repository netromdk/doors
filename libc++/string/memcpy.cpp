#include <string.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t num) {
  for (size_t i = 0; i < num; i++) {
    ((uint8_t*) dst)[i] = ((uint8_t*) src)[i];
  }
  return dst;
}
