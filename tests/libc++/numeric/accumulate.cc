#include <doctest/doctest.h>
#include <numeric>

TEST_CASE("accumulate int array")
{
  const int arr[5] = {1, 2, 3, 4, 5};
  const auto result = accumulate(arr, arr + 5, 0);
  CHECK(result == 15);
}

TEST_CASE("accumulate with non-zero init")
{
  const int arr[3] = {10, 20, 30};
  const auto result = accumulate(arr, arr + 3, 100);
  CHECK(result == 160);
}

TEST_CASE("accumulate empty range")
{
  const int arr[3] = {1, 2, 3};
  const auto result = accumulate(arr, arr, 99);
  CHECK(result == 99);
}

TEST_CASE("accumulate single element")
{
  const int arr[1] = {42};
  const auto result = accumulate(arr, arr + 1, 0);
  CHECK(result == 42);
}

TEST_CASE("accumulate unsigned char checksum pattern")
{
  unsigned char bytes[] = {0x10, 0x20, 0x30, 0x40};
  auto sum = accumulate(bytes, bytes + 4, (unsigned char) 0);
  CHECK(sum == (unsigned char) 0xA0);
}

TEST_CASE("accumulate with larger type")
{
  const int arr[4] = {1000, 2000, 3000, 4000};
  const long result = accumulate(arr, arr + 4, 0L);
  CHECK(result == 10000L);
}

TEST_CASE("accumulate partial range")
{
  const int arr[5] = {1, 2, 3, 4, 5};
  const auto result = accumulate(arr + 1, arr + 4, 0);
  CHECK(result == 9); // 2 + 3 + 4
}

TEST_CASE("accumulate is constexpr")
{
  constexpr const int arr[3] = {5, 10, 15};
  constexpr const auto result = accumulate(arr, arr + 3, 0);
  CHECK(result == 30);
}
