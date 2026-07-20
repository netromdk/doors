#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("tolower")
{
  CHECK(tolower('A') == 'a');
  CHECK(tolower('G') == 'g');
  CHECK(tolower('a') == 'a');
  CHECK(tolower('g') == 'g');
}
