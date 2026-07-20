#include <doctest/doctest.h>
#include <cstdlib>

TEST_CASE("abs")
{
  CHECK(abs(-10) == 10);
  CHECK(abs(10) == 10);
  CHECK(abs(0) == 0);
}
