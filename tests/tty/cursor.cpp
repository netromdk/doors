#include <doctest/doctest.h>
#include <stdint.h>

#include <kernel/Tty.h>
#include <kernel/Vga.h>

// VGA_RAM defined in vga_ram.cpp.

TEST_CASE("cursorSetPos_updates_row_and_col")
{
  Tty::cls();
  Tty::cursorSetPos(10, 20);
  CHECK(Tty::getRow() == 10);
  CHECK(Tty::getCol() == 20);
}

TEST_CASE("cursorSetPos_zero_zero")
{
  Tty::cls();
  Tty::cursorSetPos(10, 10);
  Tty::cursorSetPos(0, 0);
  CHECK(Tty::getRow() == 0);
  CHECK(Tty::getCol() == 0);
}

TEST_CASE("cursorSetPos_max_bounds")
{
  Tty::cls();
  Tty::cursorSetPos(24, 79);
  CHECK(Tty::getRow() == 24);
  CHECK(Tty::getCol() == 79);
}

TEST_CASE("cursorEnable_does_not_crash")
{
  Tty::cls();
  Tty::cursorEnable();
}

TEST_CASE("cursorDisable_does_not_crash")
{
  Tty::cls();
  Tty::cursorDisable();
}

TEST_CASE("cursorEnable_then_cursorSetPos")
{
  Tty::cls();
  Tty::cursorEnable();
  Tty::cursorSetPos(5, 15);
  CHECK(Tty::getRow() == 5);
  CHECK(Tty::getCol() == 15);
}

TEST_CASE("cursorDisable_then_cursorSetPos")
{
  Tty::cls();
  Tty::cursorDisable();
  Tty::cursorSetPos(3, 7);
  CHECK(Tty::getRow() == 3);
  CHECK(Tty::getCol() == 7);
}
