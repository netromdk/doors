#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isdigit") {
  for (int i = 0; i < 9; i++) {
    CHECK(isdigit(i + '1'));
  }
  CHECK_FALSE(isdigit('a'));
}
