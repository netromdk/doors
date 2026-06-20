#include <algorithm.h>
#include <doctest/doctest.h>

TEST_CASE("max")
{
  CHECK(max(1, 2) == 2);
  CHECK(max(100, 20) == 100);
}
