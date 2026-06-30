#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("fill int array")
{
  int arr[5] = {1, 2, 3, 4, 5};
  fill(arr, arr + 5, 99);
  CHECK(arr[0] == 99);
  CHECK(arr[1] == 99);
  CHECK(arr[2] == 99);
  CHECK(arr[3] == 99);
  CHECK(arr[4] == 99);
}

TEST_CASE("fill partial range")
{
  int arr[5] = {1, 2, 3, 4, 5};
  fill(arr + 1, arr + 4, 0);
  CHECK(arr[0] == 1);
  CHECK(arr[1] == 0);
  CHECK(arr[2] == 0);
  CHECK(arr[3] == 0);
  CHECK(arr[4] == 5);
}

TEST_CASE("fill empty range")
{
  int arr[3] = {1, 2, 3};
  fill(arr, arr, 99);
  CHECK(arr[0] == 1);
  CHECK(arr[1] == 2);
  CHECK(arr[2] == 3);
}

TEST_CASE("fill_n int array")
{
  int arr[5] = {1, 2, 3, 4, 5};
  const auto result = fill_n(arr, 5, 42);
  CHECK(result == arr + 5);
  CHECK(arr[0] == 42);
  CHECK(arr[1] == 42);
  CHECK(arr[2] == 42);
  CHECK(arr[3] == 42);
  CHECK(arr[4] == 42);
}

TEST_CASE("fill_n partial")
{
  int arr[5] = {1, 2, 3, 4, 5};
  fill_n(arr + 2, 2, 77);
  CHECK(arr[0] == 1);
  CHECK(arr[1] == 2);
  CHECK(arr[2] == 77);
  CHECK(arr[3] == 77);
  CHECK(arr[4] == 5);
}

TEST_CASE("fill_n zero count")
{
  int arr[3] = {1, 2, 3};
  const auto result = fill_n(arr, 0, 99);
  CHECK(result == arr);
  CHECK(arr[0] == 1);
  CHECK(arr[1] == 2);
  CHECK(arr[2] == 3);
}

TEST_CASE("fill with string pointers")
{
  const char *arr[3] = {"a", "b", "c"};
  fill(arr, arr + 3, nullptr);
  CHECK(arr[0] == nullptr);
  CHECK(arr[1] == nullptr);
  CHECK(arr[2] == nullptr);
}

TEST_CASE("fill_n structs")
{
  struct Point {
    int x;
    int y;
  };
  Point pts[3] = {{1, 2}, {3, 4}, {5, 6}};
  fill_n(pts, 3, Point{0, 0});
  CHECK(pts[0].x == 0);
  CHECK(pts[0].y == 0);
  CHECK(pts[1].x == 0);
  CHECK(pts[2].y == 0);
}

TEST_CASE("fill constexpr")
{
  constexpr auto result = [] {
    int arr[4] = {1, 2, 3, 4};
    fill(arr, arr + 4, 99);
    return arr[0] + arr[1] + arr[2] + arr[3];
  }();
  CHECK(result == 396);
}

TEST_CASE("fill_n constexpr")
{
  constexpr auto result = [] {
    int arr[4] = {};
    fill_n(arr, 4, 42);
    return arr[0] + arr[3];
  }();
  CHECK(result == 84);
}
