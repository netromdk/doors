#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("abs") {
  CHECK(abs(-10) == 10);
  CHECK(abs(10) == 10);
  CHECK(abs(0) == 0);
}
