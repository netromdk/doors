#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("strtoull")
{
  CHECK(strtoull("123", nullptr, 10) == 123ULL);

  const char *buf = "hello 123 there";
  char *ptr = nullptr; // NOLINT(misc-const-correctness): &ptr passed as char ** to strtoull
  CHECK(strtoull(buf + 6, &ptr, 10) == 123ULL);
  CHECK(ptr == buf + 9);

  CHECK(strtoull("a", nullptr, 16) == 0xAULL);
  CHECK(strtoull("A", nullptr, 16) == 0xAULL);
  CHECK(strtoull("no text") == 0ULL);
  CHECK(strtoull("+10") == 10ULL);
  CHECK(strtoull("101", nullptr, 2) == 5ULL);
  CHECK(strtoull("10", nullptr, 8) == 8ULL);
  CHECK(strtoull("  2") == 2ULL);
  CHECK(strtoull("10") == 10ULL);
  CHECK(strtoull("010") == 8ULL);
  CHECK(strtoull("0x10") == 0x10ULL);
  CHECK(strtoull("0XB") == 0xBULL);
}
