#include <stdint.h>
#include <string.h>

int main() {
  const uint8_t buf[3] = {1, 2, 3};
  uint8_t dst[3] = {0};
  auto *res = memcpy(dst, buf, 3);
  if (res != dst) {
    return 1;
  }

  if (dst[0] != 1 || dst[1] != 2 || dst[2] != 3) {
    return 2;
  }
  
  return 0;
}
