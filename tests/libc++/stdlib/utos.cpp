#include <stdlib.h>
#include <string.h>

int main() {
  // Decimal.
  char str[64] = {0};
  uint32_t num = 1000000;
  ltos(num, str);
  if (strcmp(str, "1000000") != 0) {
    return 1;
  }

  // Hexadecimal.
  memset(str, 0, 64);
  ltos(num, str, 16);
  if (strcmp(str, "f4240") != 0) {
    return 2;
  }

  memset(str, 0, 64);
  ltos(num, str, 16, true);
  if (strcmp(str, "F4240") != 0) {
    return 3;
  }

  // Octal.
  memset(str, 0, 64);
  ltos(num, str, 8);
  if (strcmp(str, "3641100") != 0) {
    return 4;
  }

  // Binary.
  memset(str, 0, 64);
  ltos(num, str, 2);
  if (strcmp(str, "11110100001001000000") != 0) {
    return 5;
  }

  return 0;
}
