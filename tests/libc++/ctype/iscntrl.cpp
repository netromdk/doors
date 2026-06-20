#include <ctype.h>
#include <doctest/doctest.h>

TEST_CASE("iscntrl")
{
  CHECK(iscntrl(0));
  CHECK(iscntrl(7));
  CHECK(iscntrl('\t'));
  CHECK(iscntrl('\f'));
  CHECK(iscntrl('\n'));
}
