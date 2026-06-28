#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("any_of true")
{
  int arr[] = {1, 3, 5, 7, 8};
  CHECK(any_of(arr, arr + 5, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("any_of false")
{
  int arr[] = {1, 3, 5, 7};
  CHECK(!any_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("any_of empty")
{
  int arr[] = {1, 2, 3};
  CHECK(!any_of(arr, arr, [](int) { return true; }));
}
