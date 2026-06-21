#include <doctest/doctest.h>
#include <stdint.h>

#include <kernel/Tty.h>
#include <kernel/Vga.h>

// VGA_RAM defined in vga_ram.cc.

TEST_CASE("backspace_moves_col_left")
{
  Tty::cls();
  Tty::puts("", 0, 5);
  Tty::putc('\b');
  CHECK(Tty::getRow() == 0);
  CHECK(Tty::getCol() == 4);
}

TEST_CASE("backspace_at_row_start_wraps_to_prev_row")
{
  Tty::cls();
  Tty::puts("", 1, 0);
  Tty::putc('\b');
  CHECK(Tty::getRow() == 0);
  CHECK(Tty::getCol() == 79);
}

TEST_CASE("backspace_at_top_left_does_nothing")
{
  Tty::cls();
  Tty::putc('\b');
  CHECK(Tty::getRow() == 0);
  CHECK(Tty::getCol() == 0);
}

TEST_CASE("multiple_backspaces")
{
  Tty::cls();
  Tty::puts("", 3, 10);
  Tty::putc('\b');
  Tty::putc('\b');
  Tty::putc('\b');
  CHECK(Tty::getRow() == 3);
  CHECK(Tty::getCol() == 7);
}

TEST_CASE("backspace_past_row_start_wraps_multiple_rows")
{
  Tty::cls();
  Tty::puts("", 2, 1);
  Tty::putc('\b');
  Tty::putc('\b');
  CHECK(Tty::getRow() == 1);
  CHECK(Tty::getCol() == 79);
}

TEST_CASE("write_then_backspace_restores_position")
{
  Tty::cls();
  Tty::puts("", 0, 10);
  Tty::putc('A');
  CHECK(Tty::getCol() == 11);
  Tty::putc('\b');
  CHECK(Tty::getCol() == 10);
}
