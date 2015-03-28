#include <string.h>
#include <stdint.h>

void *memmove(void *dst, const void *src, size_t num) {
  uint8_t *dst_ = (uint8_t*) dst;
  const uint8_t *src_ = (uint8_t*) src;
  if (dst_ < src_) {
    for (size_t i = 0; i < num; i++) {
      dst_[i] = src_[i];
    }
  }
  else {
    for (size_t i = num; i != 0; i--) {
      dst_[i-1] = src_[i-1];
    }
  }
  return dst;
}
