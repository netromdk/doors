#include <doctest/doctest.h>
#include <cstdio>

TEST_CASE("puts")
{
  CHECK(puts("hello world") == 11);
}
