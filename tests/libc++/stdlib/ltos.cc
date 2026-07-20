#include <doctest/doctest.h>
#include <cstdlib>
#include <cstring>

TEST_CASE("ltos")
{
  char str[64]{};
  const uint64_t num = 100000000000ULL;

  // Decimal.
  ltos(num, str);
  CHECK(strcmp(str, "100000000000") == 0);

  // Hexadecimal.
  memset(str, 0, 64);
  ltos(num, str, 16);
  CHECK(strcmp(str, "174876e800") == 0);

  memset(str, 0, 64);
  ltos(num, str, 16, true);
  CHECK(strcmp(str, "174876E800") == 0);

  // Octal.
  memset(str, 0, 64);
  ltos(num, str, 8);
  CHECK(strcmp(str, "1351035564000") == 0);

  // Binary.
  memset(str, 0, 64);
  ltos(num, str, 2);
  CHECK(strcmp(str, "1011101001000011101101110100000000000") == 0);
}
