#include <doctest/doctest.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>

namespace {

constexpr size_t ALPHABET_SIZE = 26;

} // namespace

TEST_CASE("fifo")
{
  Kbd::init();
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  CHECK(Kbd::getChar() == 'a');
  CHECK(Kbd::getChar() == 'b');
  CHECK(Kbd::getChar() == 'c');
}

TEST_CASE("wrap")
{
  Kbd::init();

  // Fill buffer to capacity, drain, repeat to verify head/tail wrap-around.
  for (int cycle = 0; cycle < 3; cycle++) {
    for (size_t i = 0; i < Kbd::BUF_SIZE; i++) {
      Kbd::pushChar(static_cast<char>('A' + (cycle + i) % ALPHABET_SIZE));
    }
    for (size_t i = 0; i < Kbd::BUF_SIZE; i++) {
      CHECK(Kbd::getChar() == static_cast<char>('A' + (cycle + i) % ALPHABET_SIZE));
    }
  }
  CHECK(Kbd::charAvail() == false);
}

TEST_CASE("empty_pop")
{
  Kbd::init();
  CHECK(Kbd::charAvail() == false);
}

TEST_CASE("full_then_read")
{
  Kbd::init();
  for (size_t i = 0; i < Kbd::BUF_SIZE; i++) {
    Kbd::pushChar('x');
  }
  CHECK(Kbd::charAvail() == true);
}

TEST_CASE("interleaved")
{
  Kbd::init();
  for (int i = 0; i < 5; i++) {
    Kbd::pushChar('a' + i);
  }
  for (int i = 0; i < 3; i++) {
    Kbd::getChar();
  }
  for (int i = 0; i < 5; i++) {
    Kbd::pushChar('f' + i);
  }
  CHECK(Kbd::getChar() == 'd'); // 4th pushed = 'd'
  CHECK(Kbd::getChar() == 'e'); // 5th pushed = 'e'
  CHECK(Kbd::getChar() == 'f');
  CHECK(Kbd::getChar() == 'g');
  CHECK(Kbd::getChar() == 'h');
}

TEST_CASE("charAvail_spam")
{
  Kbd::init();
  CHECK(Kbd::charAvail() == false);

  Kbd::pushChar('x');
  CHECK(Kbd::charAvail() == true);

  Kbd::getChar();
  CHECK(Kbd::charAvail() == false);
}
