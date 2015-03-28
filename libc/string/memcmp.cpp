#include <string.h>
#include <stdint.h>

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
  auto *ptr1_ = (const uint8_t*) ptr1;
  auto *ptr2_ = (const uint8_t*) ptr2;
  for (size_t i = 0; i < num; i++) {
    if (ptr1_[i] > ptr2_[i]) {
      return 1;
    }
    else if (ptr1_[i] < ptr2_[i]) {
      return -1;
    }    
  }
  return 0;
}
