#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("all_of true")
{
  int arr[] = {2, 4, 6, 8};
  CHECK(all_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("all_of false")
{
  int arr[] = {2, 4, 5, 8};
  CHECK(!all_of(arr, arr + 4, [](int x) { return x % 2 == 0; }));
}

TEST_CASE("all_of empty")
{
  int arr[] = {1, 2, 3};
  CHECK(all_of(arr, arr, [](int) { return false; }));
}

TEST_CASE("all_of constexpr")
{
  constexpr auto result = [] {
    const int arr[] = {2, 4, 6, 8};
    return all_of(arr, arr + 4, [](int x) { return x % 2 == 0; });
  }();
  CHECK(result);
}
