#include <doctest/doctest.h>
#include <cstring>

TEST_CASE("strstr")
{
  const char *haystack = "hello world";

  // Found at start.
  CHECK(strstr(haystack, "hello") == haystack);

  // Found in middle.
  CHECK(strstr(haystack, "world") == haystack + 6);

  // Single character.
  CHECK(strstr(haystack, " ") == haystack + 5);

  // Not found.
  CHECK(strstr(haystack, "xyz") == nullptr);

  // Empty needle returns haystack.
  CHECK(strstr(haystack, "") == haystack);

  // Empty haystack, empty needle.
  CHECK(strstr("", "") == (const char *) "");

  // Empty haystack, non-empty needle.
  CHECK(strstr("", "a") == nullptr);

  // Needle longer than haystack.
  CHECK(strstr("ab", "abc") == nullptr);

  // Partial match but not full.
  CHECK(strstr("abcde", "abd") == nullptr);

  // Match at very end.
  const char *haystack2 = "abcdef";
  CHECK(strstr(haystack2, "def") == haystack2 + 3);
}
