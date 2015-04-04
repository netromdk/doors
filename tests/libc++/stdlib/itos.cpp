#include <stdlib.h>
#include <string.h>

int main() {
  // Decimal.
  char str[64] = {0};
  itos(10, str);
  if (strcmp(str, "10") != 0) {
    return 1;
  }

  memset(str, 0, 64); // clear just in case.
  itos(-10, str);
  if (strcmp(str, "-10") != 0) {
    return 2;
  }

  // Hexadecimal.
  memset(str, 0, 64);
  itos(10, str, 16);
  if (strcmp(str, "a") != 0) {
    return 3;
  }

  memset(str, 0, 64);
  itos(-10, str, 16);
  if (strcmp(str, "-a") != 0) {
    return 4;
  }

  // Octal.
  memset(str, 0, 64);
  itos(10, str, 8);
  if (strcmp(str, "12") != 0) {
    return 5;
  }

  memset(str, 0, 64);
  itos(-10, str, 8);
  if (strcmp(str, "-12") != 0) {
    return 6;
  }

  // Binary.
  memset(str, 0, 64);
  itos(10, str, 2);
  if (strcmp(str, "1010") != 0) {
    return 7;
  }

  memset(str, 0, 64);
  itos(-10, str, 2);
  if (strcmp(str, "-1010") != 0) {
    return 8;
  }

  return 0;
}
