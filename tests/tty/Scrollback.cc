#include <cstdint>
#include <cstring>

#include <kernel/Tty.h>
#include <kernel/Vga.h>

#include "TtyFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(TtyFixture, "scrollback_empty_initially")
{
  CHECK(Tty::scrollbackSize() == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_captures_scrolled_lines")
{

  // Write 30 newline-terminated lines: 25 to fill the screen and 5 to scroll off.
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  // The most recent scrolled line should contain 'x'.
  CHECK(Tty::scrollbackLine(0) != nullptr);

  // Check that the buffer starts with the saved character.
  CHECK(Tty::scrollbackLine(0)[0] == 'x');
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_multiple_lines")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("ab\n");
  }

  // With 30 iterations and 25 visible rows, at least 5 lines scrolled off.
  REQUIRE(Tty::scrollbackSize() >= 5);
  CHECK(Tty::scrollbackLine(0)[0] == 'a');
  CHECK(Tty::scrollbackLine(0)[1] == 'b');
}

TEST_CASE_FIXTURE(TtyFixture, "scrollbackLine_returns_null_for_invalid")
{
  const int size = Tty::scrollbackSize();
  CHECK(Tty::scrollbackLine(size) == nullptr); // one past end
  CHECK(Tty::scrollbackLine(size + 9999) == nullptr);
  CHECK(Tty::scrollbackLine(-1) == nullptr);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_show_and_exit")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  Tty::scrollbackShow(1);
  CHECK(Tty::scrollbackActive());

  Tty::scrollbackExit();
  CHECK(!Tty::scrollbackActive());
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_show_zero_does_nothing")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  Tty::scrollbackShow(0);
  CHECK(!Tty::scrollbackActive());
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_putc_exits_view")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  Tty::scrollbackShow(1);
  REQUIRE(Tty::scrollbackActive());

  Tty::putc('a');
  CHECK(!Tty::scrollbackActive());
}

TEST_CASE_FIXTURE(TtyFixture, "scrollbackOffset_returns_zero_inactive")
{
  CHECK(Tty::scrollbackOffset() == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollbackOffset_after_show")
{

  // Generate enough scrollback so that `offset=3` stays unadjusted since it needs
  // `scrollbackCount_ >= 3 + SCROLLBACK_VIEW_HEIGHT - 1` = 26.
  for (int i = 0; i < 55; i++) {
    Tty::puts("x\n");
  }

  REQUIRE(Tty::scrollbackSize() >= 27);

  Tty::scrollbackShow(3);
  CHECK(Tty::scrollbackOffset() == 3);

  Tty::scrollbackExit();
  CHECK(Tty::scrollbackOffset() == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollbackOffset_after_pageUp_pageDown")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() >= 5);

  // PageUp from inactive state jumps by `SCROLLBACK_VIEW_HEIGHT` (24) and clamps to max.
  Tty::scrollbackPageUp();
  CHECK(Tty::scrollbackActive());
  const int offset1 = Tty::scrollbackOffset();
  CHECK(offset1 > 0);

  // PageDown reduces offset.
  Tty::scrollbackPageDown();
  const int offset2 = Tty::scrollbackOffset();
  CHECK(offset2 < offset1);

  // Successive PageDown eventually exits and offset goes to 0.
  while (Tty::scrollbackActive()) {
    Tty::scrollbackPageDown();
  }
  CHECK(Tty::scrollbackOffset() == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_home_goes_to_beginning")
{

  // Generate 3 pages of scrollback.
  for (size_t i = 0; i < (3 * VGA_HEIGHT) + 5; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() >= 55);

  // Enter scrollback mode at offset 1 (newest).
  Tty::scrollbackShow(1);
  REQUIRE(Tty::scrollbackActive());
  const int beforeHome = Tty::scrollbackOffset();

  // Home should go to the oldest content.
  Tty::scrollbackHome();
  CHECK(Tty::scrollbackActive());
  const int afterHome = Tty::scrollbackOffset();
  CHECK(afterHome > beforeHome);

  // PageDown from home should go to a different page.
  Tty::scrollbackPageDown();
  CHECK(Tty::scrollbackOffset() < afterHome);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_end_exits_view")
{
  for (int i = 0; i < 30; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  Tty::scrollbackShow(1);
  REQUIRE(Tty::scrollbackActive());

  // End should exit scrollback.
  Tty::scrollbackExit();
  CHECK(!Tty::scrollbackActive());
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_status_indicator")
{
  for (int i = 0; i < 60; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() >= 35);

  Tty::scrollbackShow(3);
  REQUIRE(Tty::scrollbackActive());

  // Row 0 should contain the status indicator.
  uint16_t entry = VGA_RAM[(0 * VGA_WIDTH) + 0];
  const char first = static_cast<char>(entry & 0xFF);
  CHECK(first == '-');
  entry = VGA_RAM[(0 * VGA_WIDTH) + 1];
  CHECK(static_cast<char>(entry & 0xFF) == '-');

  Tty::scrollbackExit();
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_line_up_down")
{
  for (int i = 0; i < 60; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() >= 35);

  Tty::scrollbackShow(10);
  REQUIRE(Tty::scrollbackActive());
  int initial = Tty::scrollbackOffset();

  // Line up should increase the offset.
  Tty::scrollbackLineUp();
  CHECK(Tty::scrollbackActive());
  CHECK(Tty::scrollbackOffset() == initial + 1);

  // Line down should decrease the offset.
  Tty::scrollbackLineDown();
  CHECK(Tty::scrollbackActive());
  CHECK(Tty::scrollbackOffset() == initial);

  // Line down at the bottom should exit.
  while (Tty::scrollbackActive()) {
    Tty::scrollbackLineDown();
  }
  CHECK(Tty::scrollbackOffset() == 0);
}

TEST_CASE_FIXTURE(TtyFixture, "scrollback_show_saves_and_restores_screen")
{

  // Generate some scrollback so that `scrollbackShow()` has content to display.
  for (size_t i = 0; i < VGA_HEIGHT + 5; i++) {
    Tty::puts("x\n");
  }
  REQUIRE(Tty::scrollbackSize() > 0);

  // Capture screen state just before entering scrollback mode, which is the state that
  // `scrollbackShow()` will save internally.
  uint16_t before[VGA_HEIGHT * VGA_WIDTH];
  memcpy(before, VGA_RAM, sizeof(before));

  // Enter scrollback mode this saves the screen and overwrites `VGA_RAM`.
  Tty::scrollbackShow(1);
  REQUIRE(Tty::scrollbackActive());

  // `VGA_RAM` should now differ from the saved state.
  uint16_t during[VGA_HEIGHT * VGA_WIDTH];
  memcpy(during, VGA_RAM, sizeof(during));
  CHECK(memcmp(before, during, sizeof(before)) != 0);

  // Exit scrollback `VGA_RAM` should be restored to the saved state.
  Tty::scrollbackExit();
  CHECK(!Tty::scrollbackActive());

  uint16_t after[VGA_HEIGHT * VGA_WIDTH];
  memcpy(after, VGA_RAM, sizeof(after));
  CHECK(memcmp(before, after, sizeof(before)) == 0);
}
