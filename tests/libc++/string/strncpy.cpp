#include <string.h>

int main() {
  const char *msg = "hello world";

  char buf[64] = {0};
  size_t num = 6;
  char *res = strncpy(buf, msg, num);
  if (res != buf) {
    return 1;
  }

  if (strlen(buf) != num) {
    return 2;
  }

  if (strcmp(buf, msg) != 0) {
    return 3;
  }

  // Copy shorter string than bytes to copy. Check for padding.
  char buf2[64]; // Uninitialized.
  num = 20; // 20 > 11
  res = strncpy(buf2, msg, num);
  if (res != buf2) {
    return 4;
  }

  // Since buf2 is uninitialized then pad after copying the length of
  // msg, which is 11 and keep going until 20 bytes. This means that
  // the length will indeed be 11 == 11. If strcpy() was used instead
  // then the extra bytes would not get zero-padded and thus the
  // length would be different each time (garbage data).
  if (strlen(buf2) != strlen(msg)) {
    return 5;
  }

  return 0;
}
