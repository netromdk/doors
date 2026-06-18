#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isxdigit") {
  for (int i = 0; i < 9; i++) {
    CHECK(isxdigit(i + '1'));
  }
  CHECK(isxdigit('a'));
  CHECK(isxdigit('e'));
  CHECK(isxdigit('F'));
}
