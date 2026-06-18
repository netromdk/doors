#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strlen") {
  CHECK(strlen("hello") == 5U);
  CHECK(strlen("hello  ") == 7U);
  CHECK(strlen("") == 0U);
}
