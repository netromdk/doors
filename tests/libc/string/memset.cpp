#include <stdint.h>
#include <string.h>

int main() {
  uint8_t val = 42;
  uint8_t buf[3] = {1, 2, 3};
  memset(buf, val, 3);
  if (buf[0] != val || buf[1] != val || buf[2] != val) {
    return 1;
  }

  return 0;
}
