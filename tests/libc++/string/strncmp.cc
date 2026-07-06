#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strncmp")
{
  CHECK(strncmp("hello", "hello", 2) == 0);

  // Compares using the minimum length of inputs and 200 = 5.
  CHECK(strncmp("hello", "hello", 200) == 0);

  CHECK(strncmp("1", "a", 1) < 0);
  CHECK(strncmp("a", "1", 1) > 0);

  // Length differential: shorter string is a prefix.
  CHECK(strncmp("abc", "abcdef", 6) < 0);
  CHECK(strncmp("abcdef", "abc", 6) > 0);

  // Limited to num.
  CHECK(strncmp("abc", "abcdef", 3) == 0);
  CHECK(strncmp("abcdef", "abc", 3) == 0);

  // num=0 always returns 0.
  CHECK(strncmp("anything", "anything", 0) == 0);
  CHECK(strncmp("abc", "xyz", 0) == 0);

  // Empty string vs. non-empty.
  CHECK(strncmp("", "a", 1) < 0);
  CHECK(strncmp("a", "", 1) > 0);
  CHECK(strncmp("", "", 1) == 0);
}
