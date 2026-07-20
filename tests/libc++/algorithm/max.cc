#include <algorithm.h>
#include <doctest/doctest.h>

TEST_CASE("max")
{
  CHECK(max(1, 2) == 2);
  CHECK(max(100, 20) == 100);

  const int a = 10;
  const int b = 20;
  CHECK(&max(a, b) == &b);
  CHECK(max(a, b) == 20);
}
