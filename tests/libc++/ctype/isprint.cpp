#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isprint") {
  CHECK(isprint(' '));
  CHECK(isprint('a'));
  CHECK(isprint('!'));
  CHECK(isprint('='));
  CHECK(isprint('Z'));
}
