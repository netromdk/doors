#include <doctest/doctest.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>

TEST_CASE("letter_lower")
{
  CHECK(KeyMap::toText(Key::A, false, false, false) == 'a');
}
TEST_CASE("letter_upper")
{
  CHECK(KeyMap::toText(Key::A, true, false, false) == 'A');
}
TEST_CASE("caps_lock")
{
  CHECK(KeyMap::toText(Key::A, false, false, true) == 'A');
}
TEST_CASE("caps_shift")
{
  CHECK(KeyMap::toText(Key::A, true, false, true) == 'a');
}
TEST_CASE("digit")
{
  CHECK(KeyMap::toText(Key::_1, false, false, false) == '1');
}
TEST_CASE("digit_shift")
{
  CHECK(KeyMap::toText(Key::_1, true, false, false) == '!');
}
TEST_CASE("ctrl_a")
{
  CHECK(KeyMap::toText(Key::A, false, true, false) == 0x01);
}
TEST_CASE("unknown")
{
  CHECK(KeyMap::toText(Key::Unknown, false, false, false) == 0);
}
