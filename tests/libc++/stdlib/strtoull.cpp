#include <stdlib.h>

int main() {
  if (strtoull("123", nullptr, 10) != 123) {
    return 1;
  }

  const char *buf = "hello 123 there";
  char *ptr = nullptr;
  if (strtoull(buf + 6, &ptr, 10) != 123) {
    return 2;
  }

  // 'ptr' must point to right after the number found.
  if (!ptr || ptr != buf + 9) {
    return 3;
  }

  // Interpret hexadecimal.
  if (strtoull("a", nullptr, 16) != 0xA) {
    return 4;
  }

  // Case doesn't matter.
  if (strtoull("A", nullptr, 16) != 0xA) {
    return 5;
  }

  // When not detecting a number then expect zero.
  if (strtoull("no text") != 0) {
    return 6;
  }
  
  // Respect sign.
  if (strtoull("+10") != 10) {
    return 7;
  }

  // Interpret binary.
  if (strtoull("101", nullptr, 2) != 5) {
    return 8;
  }

  // Interpret octal.
  if (strtoull("10", nullptr, 8) != 8) {
    return 9;
  }

  // Ignore any whitespace preceding the first digit.
  if (strtoull("  2") != 2) {
    return 10;
  }

  // Using base zero should auto-detect the base. If it could not then
  // default to decimal base 10.
  if (strtoull("10") != 10) {
    return 11;
  }

  // Auto-detect octal base.
  if (strtoull("010") != 8) {
    return 12;
  }

  // Auto-detect hexadecimal base.
  if (strtoull("0x10") != 0x10) {
    return 13;
  }

  if (strtoull("0XB") != 0xB) {
    return 14;
  }

  return 0;
}
