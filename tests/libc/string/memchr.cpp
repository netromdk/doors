#include <stdint.h>
#include <string.h>

int main() {
  const uint8_t buf[3] = {1, 2, 3};
  auto *res = memchr(buf, 2, 3);
  if (res != buf + 1) {
    return 1;
  }

  res = memchr(buf, 3, 3);
  if (res != buf + 2) {
    return 2;
  }

  // Search for something nonexistent.
  res = memchr(buf, 42, 3);
  if (res != NULL) {
    return 3;
  }

  return 0;
}
