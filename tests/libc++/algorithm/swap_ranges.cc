#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("swap_ranges int arrays")
{
  int a[4] = {1, 2, 3, 4};
  int b[4] = {5, 6, 7, 8};
  const auto result = swap_ranges(a, a + 4, b);
  CHECK(result == b + 4);

  CHECK(a[0] == 5);
  CHECK(a[1] == 6);
  CHECK(a[2] == 7);
  CHECK(a[3] == 8);

  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
  CHECK(b[2] == 3);
  CHECK(b[3] == 4);
}

TEST_CASE("swap_ranges partial")
{
  int a[4] = {1, 2, 3, 4};
  int b[4] = {5, 6, 7, 8};
  swap_ranges(a + 1, a + 3, b + 1);

  CHECK(a[0] == 1);
  CHECK(a[1] == 6);
  CHECK(a[2] == 7);
  CHECK(a[3] == 4);

  CHECK(b[0] == 5);
  CHECK(b[1] == 2);
  CHECK(b[2] == 3);
  CHECK(b[3] == 8);
}

TEST_CASE("swap_ranges empty")
{
  int a[3] = {1, 2, 3};
  int b[3] = {4, 5, 6};
  const auto result = swap_ranges(a, a, b);
  CHECK(result == b);
  CHECK(a[0] == 1);
  CHECK(b[0] == 4);
}

TEST_CASE("swap_ranges constexpr")
{
  constexpr auto result = [] {
    int a[2] = {1, 2};
    int b[2] = {3, 4};
    swap_ranges(a, a + 2, b);
    return a[0] + a[1] + b[0] + b[1];
  }();
  CHECK(result == 10);
}
