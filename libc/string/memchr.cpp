#include <string.h>
#include <stdint.h>

const void *memchr(const void *ptr, int value, size_t num) {
  auto *ptr_ = (const uint8_t*) ptr;
  for (size_t i = 0; i < num; i++) {
    if (ptr_[i] == (uint8_t) value) {
      return ptr_+i;
    }
  }
  return NULL;
}
