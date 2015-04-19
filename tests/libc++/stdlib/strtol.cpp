#include <stdlib.h>

int main() {
  if (strtol("123", nullptr, 10) != 123) {
    return 1;
  }

  const char *buf = "hello 123 there";
  char *ptr = nullptr;
  if (strtol(buf + 6, &ptr, 10) != 123) {
    return 2;
  }

  // 'ptr' must point to right after the number found.
  if (!ptr || ptr != buf + 9) {
    return 3;
  }

  // Interpret hexadecimal.
  if (strtol("a", nullptr, 16) != 0xA) {
    return 4;
  }

  // Case doesn't matter.
  if (strtol("A", nullptr, 16) != 0xA) {
    return 5;
  }

  // When not detecting a number then expect zero.
  if (strtol("no text") != 0) {
    return 6;
  }
  
  // Respect sign.
  if (strtol("-10") != -10) {
    return 7;
  }

  if (strtol("+10") != 10) {
    return 8;
  }

  // Interpret binary.
  if (strtol("101", nullptr, 2) != 5) {
    return 9;
  }

  // Interpret octal.
  if (strtol("10", nullptr, 8) != 8) {
    return 10;
  }

  // Ignore any whitespace preceding the first digit.
  if (strtol("  2") != 2) {
    return 11;
  }

  // Using base zero should auto-detect the base. If it could not then
  // default to decimal base 10.
  if (strtol("10") != 10) {
    return 12;
  }

  // Auto-detect octal base.
  if (strtol("010") != 8) {
    return 13;
  }

  // Auto-detect hexadecimal base.
  if (strtol("0x10") != 0x10) {
    return 14;
  }

  if (strtol("0XB") != 0xB) {
    return 15;
  }

  return 0;
}
