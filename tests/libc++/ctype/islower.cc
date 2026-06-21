#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("islower")
{
  CHECK(islower('a'));
  CHECK(islower('g'));
  CHECK_FALSE(islower('A'));
  CHECK_FALSE(islower('G'));
}
