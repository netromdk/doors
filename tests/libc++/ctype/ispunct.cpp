#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("ispunct")
{
  CHECK(ispunct('!'));
  CHECK(ispunct('='));
  CHECK(ispunct(']'));
  CHECK(ispunct('|'));
  CHECK(ispunct('.'));
  CHECK(ispunct(','));
}
