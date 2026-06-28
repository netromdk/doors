#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("find finds existing element")
{
  const int arr[] = {1, 2, 3, 4, 5};
  const auto it = find(arr, arr + 5, 3);
  CHECK(*it == 3);
}

TEST_CASE("find returns last when not found")
{
  const int arr[] = {1, 2, 3, 4, 5};
  const auto it = find(arr, arr + 5, 99);
  CHECK(it == arr + 5);
}

TEST_CASE("find first element")
{
  const int arr[] = {10, 20, 30};
  const auto it = find(arr, arr + 3, 10);
  CHECK(it == arr);
  CHECK(*it == 10);
}

TEST_CASE("find last element")
{
  const int arr[] = {10, 20, 30};
  const auto it = find(arr, arr + 3, 30);
  CHECK(it == arr + 2);
  CHECK(*it == 30);
}

TEST_CASE("find empty range")
{
  const int arr[] = {1, 2, 3};
  const auto it = find(arr, arr, 1);
  CHECK(it == arr);
}

TEST_CASE("find with array")
{
  const int arr[] = {5, 10, 15, 20, 25};
  const auto it = find(arr, arr + 5, 15);
  CHECK(it == arr + 2);
  CHECK(*it == 15);
}

TEST_CASE("find_if finds matching element")
{
  const int arr[] = {1, 3, 5, 7, 8, 9};
  const auto it = find_if(arr, arr + 6, [](int x) { return x % 2 == 0; });
  CHECK(it == arr + 4);
  CHECK(*it == 8);
}

TEST_CASE("find_if returns last when no match")
{
  const int arr[] = {1, 3, 5, 7};
  const auto it = find_if(arr, arr + 4, [](int x) { return x % 2 == 0; });
  CHECK(it == arr + 4);
}

TEST_CASE("find_if first element matches")
{
  const int arr[] = {2, 3, 5, 7};
  const auto it = find_if(arr, arr + 4, [](int x) { return x % 2 == 0; });
  CHECK(it == arr);
  CHECK(*it == 2);
}

TEST_CASE("find_if last element matches")
{
  const int arr[] = {1, 3, 5, 7, 10};
  const auto it = find_if(arr, arr + 5, [](int x) { return x > 9; });
  CHECK(it == arr + 4);
  CHECK(*it == 10);
}

TEST_CASE("find_if empty range")
{
  const int arr[] = {1, 2, 3};
  const auto it = find_if(arr, arr, [](int x) { return x == 1; });
  CHECK(it == arr);
}

TEST_CASE("find_if pointer to member")
{
  struct Point {
    int x;
    int y;
  };

  Point pts[] = {{1, 2}, {3, 4}, {5, 6}};
  const auto it = find_if(pts, pts + 3, [](const Point &p) { return p.x == 3; });
  CHECK(it == pts + 1);
  CHECK(it->y == 4);

  auto notFound = find_if(pts, pts + 3, [](const Point &p) { return p.x == 99; });
  CHECK(notFound == pts + 3);
}
