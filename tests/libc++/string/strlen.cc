#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strlen")
{
  CHECK(strlen("hello") == 5U);
  CHECK(strlen("hello  ") == 7U);
  CHECK(strlen("") == 0U);
  CHECK(strlen("a") == 1U);
  CHECK(strlen("abcdefghij") == 10U);
}

TEST_CASE("strlen constexpr")
{
  constexpr auto result = [] {
    return strlen("hello") == 5U && strlen("") == 0U && strlen("a") == 1U &&
           strlen("abcdefghij") == 10U;
  }();
  CHECK(result);
}
