#include <doctest/doctest.h>
#include <cstdlib>

TEST_CASE("atol")
{
  CHECK(atol("123") == 123);
  CHECK(atol("    123") == 123);
  CHECK(atol("  a  ") == 0);
}
