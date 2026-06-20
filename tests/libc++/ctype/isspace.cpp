#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("isspace")
{
  CHECK(isspace(' '));
  CHECK(isspace('\v'));
  CHECK(isspace('\f'));
  CHECK(isspace('\t'));
}
