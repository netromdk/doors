#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isspace") {
  CHECK(isspace(' '));
  CHECK(isspace('\v'));
  CHECK(isspace('\f'));
  CHECK(isspace('\t'));
}
