#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("atoi") {
  CHECK(atoi("123") == 123);
  CHECK(atoi("    123") == 123);
  CHECK(atoi("  a  ") == 0);
}
