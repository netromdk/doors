#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("ispunct") {
  CHECK(ispunct('!'));
  CHECK(ispunct('='));
  CHECK(ispunct(']'));
  CHECK(ispunct('|'));
  CHECK(ispunct('.'));
  CHECK(ispunct(','));
}
