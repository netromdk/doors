#include <doctest/doctest.h>
#include <stdio.h>

TEST_CASE("putchar") {
  CHECK(putchar('a') == 'a');
}
