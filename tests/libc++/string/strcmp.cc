#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strcmp")
{
  CHECK(strcmp("hello", "hello") == 0);

  // t < y
  CHECK(strcmp("hi there", "hi you") < 0);

  // C > c
  CHECK(strcmp("abC", "abc") < 0);

  // Length differential: "abc" is shorter than "abcdef".
  CHECK(strcmp("abc", "abcdef") < 0);
  CHECK(strcmp("abcdef", "abc") > 0);

  // Length differential where prefix matches.
  CHECK(strcmp("", "a") < 0);
  CHECK(strcmp("a", "") > 0);
  CHECK(strcmp("", "") == 0);

  // Different at first char.
  CHECK(strcmp("abc", "xyz") < 0);
  CHECK(strcmp("xyz", "abc") > 0);
}
