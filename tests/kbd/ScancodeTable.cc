#include <doctest/doctest.h>
#include <kernel/Keymap.h>

TEST_CASE("letter_keys")
{
  CHECK(KeyMap::scancodeToKey(0x10, false) == Key::Q);
  CHECK(KeyMap::scancodeToKey(0x11, false) == Key::W);
  CHECK(KeyMap::scancodeToKey(0x12, false) == Key::E);
  CHECK(KeyMap::scancodeToKey(0x13, false) == Key::R);
  CHECK(KeyMap::scancodeToKey(0x14, false) == Key::T);
  CHECK(KeyMap::scancodeToKey(0x15, false) == Key::Y);
  CHECK(KeyMap::scancodeToKey(0x16, false) == Key::U);
  CHECK(KeyMap::scancodeToKey(0x17, false) == Key::I);
  CHECK(KeyMap::scancodeToKey(0x18, false) == Key::O);
  CHECK(KeyMap::scancodeToKey(0x19, false) == Key::P);
}

TEST_CASE("number_keys")
{
  CHECK(KeyMap::scancodeToKey(0x02, false) == Key::_1);
  CHECK(KeyMap::scancodeToKey(0x03, false) == Key::_2);
  CHECK(KeyMap::scancodeToKey(0x04, false) == Key::_3);
  CHECK(KeyMap::scancodeToKey(0x05, false) == Key::_4);
  CHECK(KeyMap::scancodeToKey(0x06, false) == Key::_5);
  CHECK(KeyMap::scancodeToKey(0x07, false) == Key::_6);
  CHECK(KeyMap::scancodeToKey(0x08, false) == Key::_7);
  CHECK(KeyMap::scancodeToKey(0x09, false) == Key::_8);
  CHECK(KeyMap::scancodeToKey(0x0A, false) == Key::_9);
  CHECK(KeyMap::scancodeToKey(0x0B, false) == Key::_0);
}

TEST_CASE("enter")
{
  CHECK(KeyMap::scancodeToKey(0x1C, false) == Key::Enter);
}

TEST_CASE("backspace")
{
  CHECK(KeyMap::scancodeToKey(0x0E, false) == Key::Backspace);
}

TEST_CASE("lshift")
{
  CHECK(KeyMap::scancodeToKey(0x2A, false) == Key::LShift);
}

TEST_CASE("rshift")
{
  CHECK(KeyMap::scancodeToKey(0x36, false) == Key::RShift);
}

TEST_CASE("unknown")
{
  CHECK(KeyMap::scancodeToKey(0xFF, false) == Key::Unknown);
}

TEST_CASE("extended_arrow_up")
{
  CHECK(KeyMap::scancodeToKey(0x48, true) == Key::Up);
}

TEST_CASE("extended_arrow_left")
{
  CHECK(KeyMap::scancodeToKey(0x4B, true) == Key::Left);
}

TEST_CASE("non_extended")
{
  CHECK(KeyMap::scancodeToKey(0x48, false) == Key::Keypad8);
}
