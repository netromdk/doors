#include <doctest/doctest.h>
#include <ctype.h>

TEST_CASE("isgraph") {
  CHECK(isgraph('!'));
  CHECK(isgraph('='));
  CHECK(isgraph(']'));
  CHECK(isgraph('|'));
  CHECK(isgraph('a'));
  CHECK_FALSE(isgraph(0));
  CHECK_FALSE(isgraph('\t'));
}
