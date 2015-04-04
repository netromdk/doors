#include <stdint.h>
#include <string.h>

int main() {
  const uint8_t buf[3] = {1, 2, 3};
  const uint8_t buf2[3] = {1, 2, 3};
  if (memcmp(buf, buf2, 3) != 0) {
    return 1;
  }

  // 1 < 3
  const uint8_t buf3[3] = {3, 2, 1};
  if (memcmp(buf, buf3, 3) != -1) {
    return 2;
  }

  // 3 < 1
  if (memcmp(buf3, buf, 3) != 1) {
    return 3;
  }
  
  return 0;
}
