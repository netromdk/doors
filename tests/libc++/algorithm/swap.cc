#include <algorithm.h>
#include <doctest/doctest.h>

TEST_CASE("swap")
{
  int a = 1;
  int b = 2;
  swap(a, b);
  CHECK(a == 2);
  CHECK(b == 1);
}

TEST_CASE("swap constexpr")
{
  constexpr auto result = [] {
    int x = 10;
    int y = 20;
    swap(x, y);
    return x * 10 + y;
  }();
  CHECK(result == 210);
}
