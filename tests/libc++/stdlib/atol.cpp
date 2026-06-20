#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("atol")
{
  CHECK(atol("123") == 123);
  CHECK(atol("    123") == 123);
  CHECK(atol("  a  ") == 0);
}
