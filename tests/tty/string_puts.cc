#include <doctest/doctest.h>
#include <stdint.h>
#include <string>

#include <kernel/Tty.h>
#include <kernel/Vga.h>

// VGA_RAM defined in vga_ram.cc.

TEST_CASE("puts_string_writes_to_VGA_RAM_at_row_col")
{
  Tty::cls();
  Tty::setColor(vgaColor(COLOR_WHITE, COLOR_BLACK));
  Tty::puts(string("ab"), 2, 3);

  CHECK(VGA_RAM[2 * VGA_WIDTH + 3] == vgaEntry('a', vgaColor(COLOR_WHITE, COLOR_BLACK)));
  CHECK(VGA_RAM[2 * VGA_WIDTH + 4] == vgaEntry('b', vgaColor(COLOR_WHITE, COLOR_BLACK)));
}

TEST_CASE("puts_string_with_row_col_positions_cursor")
{
  Tty::cls();
  Tty::puts(string("hello"), 5, 10);

  CHECK(Tty::getRow() == 5);
  CHECK(Tty::getCol() == 15);
}

TEST_CASE("puts_string_advances_cursor")
{
  Tty::cls();
  Tty::cursorSetPos(3, 7);
  Tty::puts(string("xyz"));

  CHECK(Tty::getRow() == 3);
  CHECK(Tty::getCol() == 10);
}

TEST_CASE("puts_string_returns_length")
{
  Tty::cls();

  int r = Tty::puts(string(""), 0, 0);
  CHECK(r == 0);

  r = Tty::puts(string("abc"), 0, 0);
  CHECK(r == 3);

  r = Tty::puts(string("hello world"), 0, 0);
  CHECK(r == 11);
}

TEST_CASE("puts_string_with_row_col_uses_termColor")
{
  Tty::cls();
  Tty::setColor(vgaColor(COLOR_RED, COLOR_BLUE));
  Tty::puts(string("X"), 1, 1);

  CHECK(VGA_RAM[1 * VGA_WIDTH + 1] == vgaEntry('X', vgaColor(COLOR_RED, COLOR_BLUE)));
}

TEST_CASE("puts_string_empty_does_nothing")
{
  Tty::cls();
  Tty::cursorSetPos(4, 5);
  Tty::puts(string(""), 7, 3);

  CHECK(Tty::getRow() == 7);
  CHECK(Tty::getCol() == 3);
}
