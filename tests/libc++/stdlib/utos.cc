#include <doctest/doctest.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

TEST_CASE("utos")
{
  char str[64]{};

  utos(0, str);
  CHECK(strcmp(str, "0") == 0);

  memset(str, 0, 64);
  utos(10, str);
  CHECK(strcmp(str, "10") == 0);

  memset(str, 0, 64);
  utos(1000000, str);
  CHECK(strcmp(str, "1000000") == 0);

  memset(str, 0, 64);
  utos(1000000, str, 16);
  CHECK(strcmp(str, "f4240") == 0);

  memset(str, 0, 64);
  utos(1000000, str, 16, true);
  CHECK(strcmp(str, "F4240") == 0);

  memset(str, 0, 64);
  utos(1000000, str, 8);
  CHECK(strcmp(str, "3641100") == 0);

  memset(str, 0, 64);
  utos(1000000, str, 2);
  CHECK(strcmp(str, "11110100001001000000") == 0);

  memset(str, 0, 64);
  utos(UINT32_MAX, str);
  CHECK(strcmp(str, "4294967295") == 0);

  memset(str, 0, 64);
  utos(UINT32_MAX, str, 16);
  CHECK(strcmp(str, "ffffffff") == 0);

  memset(str, 0, 64);
  utos(UINT32_MAX, str, 16, true);
  CHECK(strcmp(str, "FFFFFFFF") == 0);

  memset(str, 0, 64);
  utos(UINT32_MAX, str, 2);
  CHECK(strcmp(str, "11111111111111111111111111111111") == 0);
}
