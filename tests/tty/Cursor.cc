#include "TtyFixture.h"

TEST_CASE_FIXTURE(TtyFixture, "cursorSetPos_updates_row_and_col")
{
  Tty::cursorSetPos(10, 20);
  CHECK(Tty::getCursor().first == 10);
  CHECK(Tty::getCursor().second == 20);
}

TEST_CASE_FIXTURE(TtyFixture, "cursorSetPos_zero_zero")
{
  Tty::cursorSetPos(10, 10);
  Tty::cursorSetPos(0, 0);
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "cursorSetPos_max_bounds")
{
  Tty::cursorSetPos(24, 79);
  CHECK(Tty::getCursor().first == 24);
  CHECK(Tty::getCursor().second == 79);
}

TEST_CASE_FIXTURE(TtyFixture, "cursorEnable_does_not_crash")
{
  Tty::cursorEnable();
}

TEST_CASE_FIXTURE(TtyFixture, "cursorDisable_does_not_crash")
{
  Tty::cursorDisable();
}

TEST_CASE_FIXTURE(TtyFixture, "cursorEnable_then_cursorSetPos")
{
  Tty::cursorEnable();
  Tty::cursorSetPos(5, 15);
  CHECK(Tty::getCursor().first == 5);
  CHECK(Tty::getCursor().second == 15);
}

TEST_CASE_FIXTURE(TtyFixture, "cursorDisable_then_cursorSetPos")
{
  Tty::cursorDisable();
  Tty::cursorSetPos(3, 7);
  CHECK(Tty::getCursor().first == 3);
  CHECK(Tty::getCursor().second == 7);
}
