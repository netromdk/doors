#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isalpha") {
  CHECK(isalpha('A'));
  CHECK(isalpha('H'));
  CHECK(isalpha('a'));
  CHECK(isalpha('h'));
  CHECK_FALSE(isalpha('!'));
}
