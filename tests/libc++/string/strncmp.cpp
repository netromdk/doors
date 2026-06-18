#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strncmp") {
  CHECK(strncmp("hello", "hello", 2) == 0);

  // Compares using the minimum length of inputs and 200 = 5.
  CHECK(strncmp("hello", "hello", 200) == 0);

  CHECK(strncmp("1", "a", 1) == -1);
  CHECK(strncmp("a", "1", 1) == 1);
}
