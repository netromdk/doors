#include <string.h>
#include <stdint.h>

const void *memchr(const void *ptr, int value, size_t num) {
  for (size_t i = 0; i < num; i++) {
    if (((uint8_t*) ptr)[i] == (uint8_t) value) {
      return ptr+i;
    }
  }
  return NULL;
}
