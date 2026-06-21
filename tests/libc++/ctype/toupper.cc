#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("toupper")
{
  CHECK(toupper('a') == 'A');
  CHECK(toupper('g') == 'G');
  CHECK(toupper('A') == 'A');
  CHECK(toupper('G') == 'G');
}
