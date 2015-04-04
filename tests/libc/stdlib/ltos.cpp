#include <stdlib.h>
#include <string.h>

int main() {
  // Decimal.
  char str[64] = {0};
  uint64_t num = 100000000000;
  ltos(num, str);
  if (strcmp(str, "100000000000") != 0) {
    return 1;
  }

  // Hexadecimal.
  memset(str, 0, 64);
  ltos(num, str, 16);
  if (strcmp(str, "174876e800") != 0) {
    return 2;
  }

  // Octal.
  memset(str, 0, 64);
  ltos(num, str, 8);
  if (strcmp(str, "1351035564000") != 0) {
    return 3;
  }

  // Binary.
  memset(str, 0, 64);
  ltos(num, str, 2);
  if (strcmp(str, "1011101001000011101101110100000000000") != 0) {
    return 4;
  }

  return 0;
}
