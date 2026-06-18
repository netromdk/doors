#include <doctest/doctest.h>
#include <stdio.h>

TEST_CASE("puts") {
  CHECK(puts("hello world") == 11);
}
