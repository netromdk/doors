#include <doctest/doctest.h>
#include <cstdlib>

TEST_CASE("atoi")
{
  CHECK(atoi("123") == 123);
  CHECK(atoi("    123") == 123);
  CHECK(atoi("  a  ") == 0);
}
