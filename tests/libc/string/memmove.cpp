#include <stdint.h>
#include <string.h>

int main() {
  uint8_t val = 42;
  const uint8_t buf[3] = {val, val, val};
  uint8_t buf2[3] = {0};
  auto *res = memmove(buf2, buf, 3);
  if (res != buf2) {
    return 1;
  }

  if (memcmp(buf, buf2, 3) != 0) {
    return 2;
  }

  // Copy from same chunk of memory to itself.
  uint8_t buf3[3] = {1, 2, 3};
  res = memmove(buf3, buf3 + 1, 2);
  if (res != buf3) {
    return 3;
  }

  const uint8_t buf3_[3] = {2, 3, 3}; // expected
  if (memcmp(buf3, buf3_, 3) != 0) {
    return 4;
  }

  return 0;
}
