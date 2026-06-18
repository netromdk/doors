#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isblank") {
  CHECK(isblank('\t'));
  CHECK(isblank(' '));
  CHECK_FALSE(isblank('a'));
}
