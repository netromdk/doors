#include "KbdFixture.h"
#include <doctest/doctest.h>
#include <kernel/Kbd.h>
#include <kernel/Scancodes.h>

TEST_CASE_FIXTURE(KbdFixture, "shift_make_and_type")
{
  Kbd::processScancode(SCANCODE_LSHIFT, false);
  CHECK(Kbd::isShiftPressed() == true);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == 'A');
}

TEST_CASE_FIXTURE(KbdFixture, "shift_break")
{
  Kbd::processScancode(SCANCODE_LSHIFT, false);
  Kbd::processScancode(SCANCODE_LSHIFT | SCANCODE_BREAK_BIT, false);
  CHECK(Kbd::isShiftPressed() == false);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "caps_toggle")
{
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCapsLock() == true);

  // Caps make again to toggle off.
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCapsLock() == false);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "both_shifts")
{
  Kbd::processScancode(SCANCODE_LSHIFT, false);
  Kbd::processScancode(SCANCODE_RSHIFT, false);
  CHECK(Kbd::isShiftPressed() == true);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == 'A');

  Kbd::processScancode(SCANCODE_LSHIFT | SCANCODE_BREAK_BIT, false);
  Kbd::processScancode(SCANCODE_RSHIFT | SCANCODE_BREAK_BIT, false);
  CHECK(Kbd::isShiftPressed() == false);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "lctrl_make_and_break")
{
  CHECK(Kbd::isCtrlPressed() == false);

  Kbd::processScancode(SCANCODE_LCTRL, false);
  CHECK(Kbd::isCtrlPressed() == true);

  Kbd::processScancode(SCANCODE_LCTRL | SCANCODE_BREAK_BIT, false);
  CHECK(Kbd::isCtrlPressed() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "lalt_make_and_break")
{
  CHECK(Kbd::isAltPressed() == false);

  Kbd::processScancode(SCANCODE_LALT, false);
  CHECK(Kbd::isAltPressed() == true);

  Kbd::processScancode(SCANCODE_LALT | SCANCODE_BREAK_BIT, false);
  CHECK(Kbd::isAltPressed() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "rctrl_make_and_break")
{
  CHECK(Kbd::isCtrlPressed() == false);

  Kbd::processScancode(SCANCODE_LCTRL, true);
  CHECK(Kbd::isCtrlPressed() == true);

  Kbd::processScancode(SCANCODE_LCTRL | SCANCODE_BREAK_BIT, true);
  CHECK(Kbd::isCtrlPressed() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "ralt_make_and_break")
{
  CHECK(Kbd::isAltPressed() == false);

  Kbd::processScancode(SCANCODE_LALT, true);
  CHECK(Kbd::isAltPressed() == true);

  Kbd::processScancode(SCANCODE_LALT | SCANCODE_BREAK_BIT, true);
  CHECK(Kbd::isAltPressed() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_up_sets_pending")
{
  Kbd::processScancode(SCANCODE_UP, true);
  CHECK(Kbd::pendingUp_ == 1);

  Kbd::processScancode(SCANCODE_UP, true);
  CHECK(Kbd::pendingUp_ == 2);

  Kbd::processScancode(SCANCODE_UP | SCANCODE_BREAK_BIT, true);
  CHECK(Kbd::pendingUp_ == 2); // Break does not increment
}

TEST_CASE_FIXTURE(KbdFixture, "extended_down_sets_pending")
{
  Kbd::processScancode(SCANCODE_DOWN, true);
  CHECK(Kbd::pendingDown_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_left_sets_pending")
{
  Kbd::processScancode(SCANCODE_LEFT, true);
  CHECK(Kbd::pendingLeft_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_right_sets_pending")
{
  Kbd::processScancode(SCANCODE_RIGHT, true);
  CHECK(Kbd::pendingRight_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_pageup_sets_pending")
{
  Kbd::processScancode(SCANCODE_PAGEUP, true);
  CHECK(Kbd::pendingPageUp_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_pagedown_sets_pending")
{
  Kbd::processScancode(SCANCODE_PAGEDOWN, true);
  CHECK(Kbd::pendingPageDown_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_home_sets_pending")
{
  Kbd::processScancode(SCANCODE_HOME, true);
  CHECK(Kbd::pendingHome_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "extended_end_sets_pending")
{
  Kbd::processScancode(SCANCODE_END, true);
  CHECK(Kbd::pendingEnd_ == 1);
}

TEST_CASE_FIXTURE(KbdFixture, "shift_home_maps_to_end")
{
  Kbd::processScancode(SCANCODE_LSHIFT, false);
  CHECK(Kbd::isShiftPressed() == true);

  Kbd::processScancode(SCANCODE_HOME, true);
  CHECK(Kbd::pendingEnd_ == 1);
  CHECK(Kbd::pendingHome_ == 0);

  Kbd::processScancode(SCANCODE_LSHIFT | SCANCODE_BREAK_BIT, false);
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_ctrl_shares_single_flag")
{
  // The implementation uses a single `ctrlPressed_` bool, not per-key tracking. Both `LCTRL` and
  // `RCTRL` set the same flag.

  Kbd::processScancode(SCANCODE_LCTRL, false);
  CHECK(Kbd::isCtrlPressed() == true);

  Kbd::processScancode(SCANCODE_LCTRL, true);
  CHECK(Kbd::isCtrlPressed() == true);

  // `LCTRL` break clears the shared flag.
  Kbd::processScancode(SCANCODE_LCTRL | SCANCODE_BREAK_BIT, false);
  CHECK(Kbd::isCtrlPressed() == false);

  // `RCTRL` make + break cycle.
  Kbd::processScancode(SCANCODE_LCTRL, true);
  CHECK(Kbd::isCtrlPressed() == true);

  Kbd::processScancode(SCANCODE_LCTRL | SCANCODE_BREAK_BIT, true);
  CHECK(Kbd::isCtrlPressed() == false);
}
