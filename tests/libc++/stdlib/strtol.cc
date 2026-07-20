#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("strtol")
{
  CHECK(strtol("123", nullptr, 10) == 123);

  const char *buf = "hello 123 there";
  char *ptr = nullptr; // NOLINT(misc-const-correctness): &ptr passed as char ** to strtol
  CHECK(strtol(buf + 6, &ptr, 10) == 123);

  // 'ptr' must point to right after the number found.
  CHECK(ptr == buf + 9);

  // Interpret hexadecimal.
  CHECK(strtol("a", nullptr, 16) == 0xA);

  // Case doesn't matter.
  CHECK(strtol("A", nullptr, 16) == 0xA);

  // When not detecting a number then expect zero.
  CHECK(strtol("no text") == 0);

  // Respect sign.
  CHECK(strtol("-10") == -10);
  CHECK(strtol("+10") == 10);

  // Interpret binary.
  CHECK(strtol("101", nullptr, 2) == 5);

  // Interpret octal.
  CHECK(strtol("10", nullptr, 8) == 8);

  // Ignore any whitespace preceding the first digit.
  CHECK(strtol("  2") == 2);

  // Using base zero should auto-detect the base.
  // If it could not then default to decimal base 10.
  CHECK(strtol("10") == 10);

  // Auto-detect octal base.
  CHECK(strtol("010") == 8);

  CHECK(strtol("0x10") == 0x10);
  CHECK(strtol("0XB") == 0xB);
}
