#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("strtoul")
{
  CHECK(strtoul("123", nullptr, 10) == 123U);

  const char *buf = "hello 123 there";
  char *ptr = nullptr;
  CHECK(strtoul(buf + 6, &ptr, 10) == 123U);
  CHECK(ptr == buf + 9);

  CHECK(strtoul("a", nullptr, 16) == 0xAU);
  CHECK(strtoul("A", nullptr, 16) == 0xAU);
  CHECK(strtoul("no text") == 0U);
  CHECK(strtoul("+10") == 10U);
  CHECK(strtoul("101", nullptr, 2) == 5U);
  CHECK(strtoul("10", nullptr, 8) == 8U);
  CHECK(strtoul("  2") == 2U);
  CHECK(strtoul("10") == 10U);
  CHECK(strtoul("010") == 8U);
  CHECK(strtoul("0x10") == 0x10U);
  CHECK(strtoul("0XB") == 0xBU);
}
