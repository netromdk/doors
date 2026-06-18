#include <doctest/doctest.h>
#include <stdlib.h>
#include <string.h>

TEST_CASE("utos") {
  char str[64]{};
  uint32_t num = 1000000;

  ltos(num, str);
  CHECK(strcmp(str, "1000000") == 0);

  memset(str, 0, 64);
  ltos(num, str, 16);
  CHECK(strcmp(str, "f4240") == 0);

  memset(str, 0, 64);
  ltos(num, str, 16, true);
  CHECK(strcmp(str, "F4240") == 0);

  memset(str, 0, 64);
  ltos(num, str, 8);
  CHECK(strcmp(str, "3641100") == 0);

  memset(str, 0, 64);
  ltos(num, str, 2);
  CHECK(strcmp(str, "11110100001001000000") == 0);
}
