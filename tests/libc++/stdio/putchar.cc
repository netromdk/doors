#include <doctest/doctest.h>
#include <cstdio>

TEST_CASE("putchar")
{
  CHECK(putchar('a') == 'a');
}
