#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("copy int array")
{
  const int src[5] = {1, 2, 3, 4, 5};
  int dst[5] = {};
  const auto result = copy(src, src + 5, dst);
  CHECK(result == dst + 5);
  CHECK(dst[0] == 1);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
  CHECK(dst[3] == 4);
  CHECK(dst[4] == 5);
}

TEST_CASE("copy partial range")
{
  const int src[5] = {1, 2, 3, 4, 5};
  int dst[5] = {0, 0, 0, 0, 0};
  copy(src + 1, src + 4, dst + 1);
  CHECK(dst[0] == 0);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
  CHECK(dst[3] == 4);
  CHECK(dst[4] == 0);
}

TEST_CASE("copy empty range")
{
  const int src[3] = {1, 2, 3};
  int dst[3] = {0, 0, 0};
  const auto result = copy(src, src, dst);
  CHECK(result == dst);
  CHECK(dst[0] == 0);
}

TEST_CASE("copy non-overlapping forward")
{
  const int src[3] = {1, 2, 3};
  int dst[5] = {0, 0, 0, 0, 0};
  copy(src, src + 3, dst + 2);
  CHECK(dst[0] == 0);
  CHECK(dst[1] == 0);
  CHECK(dst[2] == 1);
  CHECK(dst[3] == 2);
  CHECK(dst[4] == 3);
}

TEST_CASE("copy_n int array")
{
  const int src[5] = {1, 2, 3, 4, 5};
  int dst[5] = {};
  const auto result = copy_n(src, 5, dst);
  CHECK(result == dst + 5);
  CHECK(dst[0] == 1);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
  CHECK(dst[3] == 4);
  CHECK(dst[4] == 5);
}

TEST_CASE("copy_n partial")
{
  const int src[5] = {1, 2, 3, 4, 5};
  int dst[5] = {0, 0, 0, 0, 0};
  copy_n(src + 2, 2, dst + 1);
  CHECK(dst[0] == 0);
  CHECK(dst[1] == 3);
  CHECK(dst[2] == 4);
  CHECK(dst[3] == 0);
  CHECK(dst[4] == 0);
}

TEST_CASE("copy_n zero count")
{
  const int src[3] = {1, 2, 3};
  int dst[3] = {0, 0, 0};
  const auto result = copy_n(src, 0, dst);
  CHECK(result == dst);
  CHECK(dst[0] == 0);
  CHECK(dst[1] == 0);
  CHECK(dst[2] == 0);
}

TEST_CASE("copy_n non-overlapping destination")
{
  const int src[5] = {1, 2, 3, 4, 5};
  int dst[5] = {0, 0, 0, 0, 0};
  copy_n(src + 2, 2, dst + 3);
  CHECK(dst[0] == 0);
  CHECK(dst[1] == 0);
  CHECK(dst[2] == 0);
  CHECK(dst[3] == 3);
  CHECK(dst[4] == 4);
}

TEST_CASE("copy is constexpr")
{
  constexpr const auto result = [] {
    const int src[3] = {10, 20, 30};
    int dst[3] = {};
    copy(src, src + 3, dst);
    return dst[0] + dst[1] + dst[2];
  }();
  CHECK(result == 60);
}

TEST_CASE("copy_n string pointers")
{
  const char *src[3] = {"a", "b", "c"};
  const char *dst[3] = {};
  copy_n(src, 3, dst);
  CHECK(dst[0] == src[0]);
  CHECK(dst[1] == src[1]);
  CHECK(dst[2] == src[2]);
}
