#include <doctest/doctest.h>
#include <cstdlib>

TEST_CASE("strtoll")
{
  CHECK(strtoll("123", nullptr, 10) == 123);

  const char *buf = "hello 123 there";
  char *ptr = nullptr; // NOLINT(misc-const-correctness): &ptr passed as char ** to strtoll
  CHECK(strtoll(buf + 6, &ptr, 10) == 123);
  CHECK(ptr == buf + 9);

  CHECK(strtoll("a", nullptr, 16) == 0xA);
  CHECK(strtoll("A", nullptr, 16) == 0xA);
  CHECK(strtoll("no text") == 0);
  CHECK(strtoll("-10") == -10);
  CHECK(strtoll("+10") == 10);
  CHECK(strtoll("101", nullptr, 2) == 5);
  CHECK(strtoll("10", nullptr, 8) == 8);
  CHECK(strtoll("  2") == 2);
  CHECK(strtoll("10") == 10);
  CHECK(strtoll("010") == 8);
  CHECK(strtoll("0x10") == 0x10);
  CHECK(strtoll("0XB") == 0xB);
}
