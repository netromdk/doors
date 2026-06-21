#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("isupper")
{
  CHECK_FALSE(isupper('a'));
  CHECK_FALSE(isupper('g'));
  CHECK(isupper('A'));
  CHECK(isupper('G'));
}
