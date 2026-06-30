#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("none_of true")
{
  int arr[] = {1, 3, 5, 7};
  CHECK(none_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("none_of false")
{
  int arr[] = {1, 3, 5, 6};
  CHECK(!none_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("none_of empty")
{
  int arr[] = {1, 2, 3};
  CHECK(none_of(arr, arr, [](int) { return true; }));
}

TEST_CASE("none_of constexpr")
{
  constexpr auto result = [] {
    const int arr[] = {1, 3, 5, 7};
    return none_of(arr, arr + 4, [](int x) { return x % 2 == 0; });
  }();
  CHECK(result);
}
