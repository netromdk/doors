#include <cctype>
#include <doctest/doctest.h>

TEST_CASE("iscntrl")
{
  // Control characters.
  CHECK(iscntrl(CTYPE_CNTRL1_START));
  CHECK(iscntrl(CTYPE_CNTRL1_START + 2));
  CHECK(iscntrl(CTYPE_CNTRL1_END));

  CHECK(iscntrl(CTYPE_CNTRL2_START));
  CHECK(iscntrl(CTYPE_CNTRL2_START + 2));
  CHECK(iscntrl(CTYPE_CNTRL2_END));

  CHECK(iscntrl(CTYPE_CNTRL3_START));
  CHECK(iscntrl(CTYPE_CNTRL3_START + 2));
  CHECK(iscntrl(CTYPE_CNTRL3_END));

  CHECK(iscntrl('\t'));
  CHECK(iscntrl(0x7F)); // DEL

  // Non-control characters must not match.
  CHECK_FALSE(iscntrl(CTYPE_SPACE));
  CHECK_FALSE(iscntrl('A'));
  CHECK_FALSE(iscntrl('z'));
  CHECK_FALSE(iscntrl('0'));
  CHECK_FALSE(iscntrl(CTYPE_DEL + 1)); // Above DEL
}
