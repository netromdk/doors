#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("toupper") {
  CHECK(toupper('a') == 'A');
  CHECK(toupper('g') == 'G');
  CHECK(toupper('A') == 'A');
  CHECK(toupper('G') == 'G');
}
