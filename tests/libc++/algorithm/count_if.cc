#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("count_if none match")
{
  const int arr[] = {1, 3, 5, 7};
  CHECK(count_if(arr, arr + 4, [](int x) { return x % 2 == 0; }) == 0);
}

TEST_CASE("count_if all match")
{
  const int arr[] = {2, 4, 6, 8};
  CHECK(count_if(arr, arr + 4, [](int x) { return x % 2 == 0; }) == 4);
}

TEST_CASE("count_if partial match")
{
  const int arr[] = {1, 2, 3, 4, 5, 6};
  CHECK(count_if(arr, arr + 6, [](int x) { return x % 2 == 0; }) == 3);
}

TEST_CASE("count_if empty range")
{
  const int arr[] = {1, 2, 3};
  CHECK(count_if(arr, arr, [](int x) { return true; }) == 0);
}

TEST_CASE("count_if with lambda predicate")
{
  const int arr[] = {10, 15, 20, 25, 30};
  CHECK(count_if(arr, arr + 5, [](int x) { return x > 20; }) == 2);
}

TEST_CASE("count_if with function pointer")
{
  auto isPositive = [](int x) { return x > 0; };
  const int arr[] = {-2, -1, 0, 1, 2};
  CHECK(count_if(arr, arr + 5, isPositive) == 2);
}

TEST_CASE("count_if char arrays")
{
  const char *words[] = {"cat", "", "dog", "", "bird"};
  CHECK(count_if(words, words + 5, [](const char *s) { return s[0] != '\0'; }) == 3);
}

TEST_CASE("count_if constexpr")
{
  constexpr const int arr[] = {1, 2, 3, 4, 5};
  constexpr auto result = count_if(arr, arr + 5, [](int x) { return x % 2 == 0; });
  CHECK(result == 2);
}
