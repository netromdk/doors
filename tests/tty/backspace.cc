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
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 4);
}

TEST_CASE("backspace_at_row_start_wraps_to_prev_row")
{
  Tty::cls();
  Tty::puts("", 1, 0);
  Tty::putc('\b');
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 79);
}

TEST_CASE("backspace_at_top_left_does_nothing")
{
  Tty::cls();
  Tty::putc('\b');
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 0);
}

TEST_CASE("multiple_backspaces")
{
  Tty::cls();
  Tty::puts("", 3, 10);
  Tty::putc('\b');
  Tty::putc('\b');
  Tty::putc('\b');
  CHECK(Tty::getCursor().first == 3);
  CHECK(Tty::getCursor().second == 7);
}

TEST_CASE("backspace_past_row_start_wraps_multiple_rows")
{
  Tty::cls();
  Tty::puts("", 2, 1);
  Tty::putc('\b');
  Tty::putc('\b');
  CHECK(Tty::getCursor().first == 1);
  CHECK(Tty::getCursor().second == 79);
}

TEST_CASE("write_then_backspace_restores_position")
{
  Tty::cls();
  Tty::puts("", 0, 10);
  Tty::putc('A');
  CHECK(Tty::getCursor().second == 11);
  Tty::putc('\b');
  CHECK(Tty::getCursor().second == 10);
}

TEST_CASE("puts_backspace_moves_col_left")
{
  Tty::cls();
  Tty::puts("", 0, 5);
  Tty::puts("\b");
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 4);
}

TEST_CASE("puts_multiple_backspaces")
{
  Tty::cls();
  Tty::puts("", 2, 10);
  Tty::puts("\b\b\b");
  CHECK(Tty::getCursor().first == 2);
  CHECK(Tty::getCursor().second == 7);
}

TEST_CASE("puts_backspace_does_not_write_to_vga")
{
  Tty::cls();
  Tty::puts("", 0, 0);
  Tty::puts("\b");
  // Backspace at (0,0) should be a no-op.
  CHECK(Tty::getCursor().first == 0);
  CHECK(Tty::getCursor().second == 0);
}
