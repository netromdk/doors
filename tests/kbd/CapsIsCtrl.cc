#include <cstdint>

#include <kernel/Kbd.h>
#include <kernel/Scancodes.h>

#include "KbdFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr uint8_t SCANCODE_CAPS_BREAK = SCANCODE_CAPS_LOCK | 0x80;

} // namespace

TEST_CASE_FIXTURE(KbdFixture, "caps_lock_make_sets_ctrl")
{
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCtrlPressed() == true);
  CHECK(Kbd::isCapsLock() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "caps_lock_break_clears_ctrl")
{
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCtrlPressed() == true);

  Kbd::processScancode(SCANCODE_CAPS_BREAK, false);
  CHECK(Kbd::isCtrlPressed() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "caps_lock_does_not_toggle_under_ctrl_remap")
{
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCapsLock() == false);

  Kbd::processScancode(SCANCODE_CAPS_BREAK, false);
  CHECK(Kbd::isCapsLock() == false);

  // Second make-break pair still doesn't toggle.
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);
  CHECK(Kbd::isCapsLock() == false);

  Kbd::processScancode(SCANCODE_CAPS_BREAK, false);
  CHECK(Kbd::isCapsLock() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "type_a_while_caps_is_ctrl_yields_ctrl_a")
{
  Kbd::processScancode(SCANCODE_CAPS_LOCK, false);

  Kbd::processScancode(SCANCODE_A, false);
  CHECK(Kbd::getChar() == '\x01'); // Ctrl+A = Start Of Heading (0x01).

  Kbd::processScancode(SCANCODE_CAPS_BREAK, false);
}
