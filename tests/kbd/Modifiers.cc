#include "KbdFixture.h"
#include <doctest/doctest.h>
#include <kernel/Kbd.h>

TEST_CASE_FIXTURE(KbdFixture, "shift_make_and_type")
{
  // LShift make
  Kbd::processScancode(0x2A, false);
  CHECK(Kbd::isShiftPressed() == true);

  // Type 'a' (scancode 0x1E)
  Kbd::processScancode(0x1E, false);
  CHECK(Kbd::getChar() == 'A');
}

TEST_CASE_FIXTURE(KbdFixture, "shift_break")
{
  // LShift make + break
  Kbd::processScancode(0x2A, false);
  Kbd::processScancode(0xAA, false); // 0x2A | 0x80 = break
  CHECK(Kbd::isShiftPressed() == false);

  // Type 'a'
  Kbd::processScancode(0x1E, false);
  CHECK(Kbd::getChar() == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "caps_toggle")
{
  // Caps make (0x3A)
  Kbd::processScancode(0x3A, false);
  CHECK(Kbd::isCapsLock() == true);

  // Caps make again (toggle off)
  Kbd::processScancode(0x3A, false);
  CHECK(Kbd::isCapsLock() == false);

  // Type 'a' with caps off
  Kbd::processScancode(0x1E, false);
  CHECK(Kbd::getChar() == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "both_shifts")
{
  // Both shifts make
  Kbd::processScancode(0x2A, false); // LShift
  Kbd::processScancode(0x36, false); // RShift
  CHECK(Kbd::isShiftPressed() == true);

  // Type 'a'
  Kbd::processScancode(0x1E, false);
  CHECK(Kbd::getChar() == 'A');

  // Both shifts break
  Kbd::processScancode(0xAA, false); // LShift break
  Kbd::processScancode(0xB6, false); // RShift break
  CHECK(Kbd::isShiftPressed() == false);

  // Type 'a'
  Kbd::processScancode(0x1E, false);
  CHECK(Kbd::getChar() == 'a');
}
