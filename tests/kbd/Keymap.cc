#include <doctest/doctest.h>
#include <kernel/Keymap.h>

namespace {

struct KeyParams {
  Key key;
  bool shift = false;
  bool ctrl = false;
  bool caps = false;
};

char toText(KeyParams p)
{
  return KeyMap::toText(p.key, p.shift, p.ctrl, p.caps);
}

} // anonymous namespace

TEST_CASE("letter_lower")
{
  CHECK(toText({.key = Key::A}) == 'a');
}

TEST_CASE("letter_upper")
{
  CHECK(toText({.key = Key::A, .shift = true}) == 'A');
}

TEST_CASE("caps_lock")
{
  CHECK(toText({.key = Key::A, .caps = true}) == 'A');
}

TEST_CASE("caps_shift")
{
  CHECK(toText({.key = Key::A, .shift = true, .caps = true}) == 'a');
}

TEST_CASE("digit")
{
  CHECK(toText({.key = Key::_1}) == '1');
}

TEST_CASE("digit_shift")
{
  CHECK(toText({.key = Key::_1, .shift = true}) == '!');
}

TEST_CASE("ctrl_a")
{
  CHECK(toText({.key = Key::A, .ctrl = true}) == 0x01); // ctrl+A = 0x41 - 0x40
}

TEST_CASE("unknown")
{
  CHECK(toText({.key = Key::Unknown}) == 0);
}

TEST_CASE("ctrl_with_non_letter")
{
  CHECK(toText({.key = Key::Space, .ctrl = true}) == 0);
}

TEST_CASE("symbol_unshifted")
{
  CHECK(toText({.key = Key::Backtick}) == '`');
}

TEST_CASE("symbol_shifted")
{
  CHECK(toText({.key = Key::Backtick, .shift = true}) == '~');
}

TEST_CASE("tab_key")
{
  CHECK(toText({.key = Key::Tab}) == '\t');
}

TEST_CASE("enter_key")
{
  CHECK(toText({.key = Key::Enter}) == '\n');
}

TEST_CASE("backspace_key")
{
  CHECK(toText({.key = Key::Backspace}) == '\b');
}

TEST_CASE("escape_key")
{
  CHECK(toText({.key = Key::Escape}) == static_cast<char>(Key::Escape));
}
