#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("isblank")
{
  CHECK(isblank('\t'));
  CHECK(isblank(' '));
  CHECK_FALSE(isblank('a'));
}
