#include <doctest/doctest.h>
#include <cstdlib>
#include <cstring>

TEST_CASE("itos")
{
  char str[64]{};

  itos(0, str);
  CHECK(strcmp(str, "0") == 0);

  memset(str, 0, 64);
  itos(10, str);
  CHECK(strcmp(str, "10") == 0);

  memset(str, 0, 64);
  itos(-10, str);
  CHECK(strcmp(str, "-10") == 0);

  memset(str, 0, 64);
  itos(10, str, 16);
  CHECK(strcmp(str, "a") == 0);

  memset(str, 0, 64);
  itos(10, str, 16, true);
  CHECK(strcmp(str, "A") == 0);

  memset(str, 0, 64);
  itos(-10, str, 16);
  CHECK(strcmp(str, "-a") == 0);

  memset(str, 0, 64);
  itos(10, str, 8);
  CHECK(strcmp(str, "12") == 0);

  memset(str, 0, 64);
  itos(-10, str, 8);
  CHECK(strcmp(str, "-12") == 0);

  memset(str, 0, 64);
  itos(10, str, 2);
  CHECK(strcmp(str, "1010") == 0);

  memset(str, 0, 64);
  itos(-10, str, 2);
  CHECK(strcmp(str, "-1010") == 0);

  memset(str, 0, 64);
  itos(INT32_MAX, str);
  CHECK(strcmp(str, "2147483647") == 0);

  memset(str, 0, 64);
  itos(INT32_MAX, str, 2);
  CHECK(strcmp(str, "1111111111111111111111111111111") == 0); // 31 ones

  memset(str, 0, 64);
  itos(INT32_MIN, str);
  CHECK(strcmp(str, "-2147483648") == 0);

  memset(str, 0, 64);
  itos(INT32_MIN, str, 2);
  CHECK(strcmp(str, "-10000000000000000000000000000000") == 0); // 1 + 31 zeros

  memset(str, 0, 64);
  itos(INT32_MIN, str, 16);
  CHECK(strcmp(str, "-80000000") == 0);
}
