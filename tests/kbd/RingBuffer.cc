#include <cstddef>

#include <kernel/Kbd.h>

#include "KbdFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr size_t ALPHABET_SIZE = 26;

} // namespace

TEST_CASE_FIXTURE(KbdFixture, "fifo")
{
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  CHECK(Kbd::getChar() == 'a');
  CHECK(Kbd::getChar() == 'b');
  CHECK(Kbd::getChar() == 'c');
}

TEST_CASE_FIXTURE(KbdFixture, "wrap")
{
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

TEST_CASE_FIXTURE(KbdFixture, "empty_pop")
{
  CHECK(Kbd::charAvail() == false);
}

TEST_CASE_FIXTURE(KbdFixture, "full_then_read")
{
  for (size_t i = 0; i < Kbd::BUF_SIZE; i++) {
    Kbd::pushChar('x');
  }
  CHECK(Kbd::charAvail() == true);
}

TEST_CASE_FIXTURE(KbdFixture, "interleaved")
{
  for (int i = 0; i < 5; i++) {
    Kbd::pushChar(static_cast<char>('a' + i));
  }
  for (int i = 0; i < 3; i++) {
    Kbd::getChar();
  }
  for (int i = 0; i < 5; i++) {
    Kbd::pushChar(static_cast<char>('f' + i));
  }
  CHECK(Kbd::getChar() == 'd'); // 4th pushed = 'd'
  CHECK(Kbd::getChar() == 'e'); // 5th pushed = 'e'
  CHECK(Kbd::getChar() == 'f');
  CHECK(Kbd::getChar() == 'g');
  CHECK(Kbd::getChar() == 'h');
}

TEST_CASE_FIXTURE(KbdFixture, "charAvail_spam")
{
  CHECK(Kbd::charAvail() == false);

  Kbd::pushChar('x');
  CHECK(Kbd::charAvail() == true);

  Kbd::getChar();
  CHECK(Kbd::charAvail() == false);
}
