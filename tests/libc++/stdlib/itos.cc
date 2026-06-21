#include <doctest/doctest.h>
#include <stdlib.h>
#include <string.h>

TEST_CASE("itos")
{
  char str[64]{};

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
}
