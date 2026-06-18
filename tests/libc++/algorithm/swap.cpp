#include <doctest/doctest.h>
#include <algorithm.h>

TEST_CASE("swap") {
  int a = 1, b = 2;
  swap(a, b);
  CHECK(a == 2);
  CHECK(b == 1);
}
