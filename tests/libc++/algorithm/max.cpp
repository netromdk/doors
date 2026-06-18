#include <doctest/doctest.h>
#include <algorithm.h>

TEST_CASE("max") {
  CHECK(max(1, 2) == 2);
  CHECK(max(100, 20) == 100);
}
