#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("isalnum")
{
  for (int i = 0; i < 9; i++) {
    CHECK(isalnum(i + '1'));
  }
  CHECK(isalnum('A'));
  CHECK(isalnum('H'));
  CHECK(isalnum('a'));
  CHECK(isalnum('h'));
}
