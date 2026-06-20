#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strcmp")
{
  CHECK(strcmp("hello", "hello") == 0);

  // t < y
  CHECK(strcmp("hi there", "hi you") == -1);

  // C > c
  CHECK(strcmp("abC", "abc") == -1);
}
