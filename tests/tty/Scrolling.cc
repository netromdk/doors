#include <kernel/Tty.h>

#include "TtyFixture.h"
#include "TtyTestAccess.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(TtyFixture, "scrolling off")
{
  Tty::setScrolling(false);
  CHECK(TtyTestAccess::scrolling() == false);
}

TEST_CASE_FIXTURE(TtyFixture, "scrolling on")
{
  Tty::setScrolling(true);
  CHECK(TtyTestAccess::scrolling() == true);
}
