#include <kernel/Kbd.h>

#include "KbdFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Unknown when buffer empty and no pending arrows")
{
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Unknown);
  CHECK(ke.ch == 0);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Up from pendingUp_ counter")
{
  Kbd::pendingUp_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Up);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Down from pendingDown_ counter")
{
  Kbd::pendingDown_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Down);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Left from pendingLeft_ counter")
{
  Kbd::pendingLeft_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Left);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Right from pendingRight_ counter")
{
  Kbd::pendingRight_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Right);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: PageUp from pendingPageUp_ counter")
{
  Kbd::pendingPageUp_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::PageUp);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: PageDown from pendingPageDown_ counter")
{
  Kbd::pendingPageDown_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::PageDown);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Home from pendingHome_ counter")
{
  Kbd::pendingHome_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Home);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: End from pendingEnd_ counter")
{
  Kbd::pendingEnd_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::End);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: drains exactly one counter per call")
{
  Kbd::pendingUp_ = 3;
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Unknown);
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: Up has priority over char buffer")
{
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('x');

  auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Up);

  // Char should be available on next call.
  ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Char);
  CHECK(ke.ch == 'x');
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: char returned")
{
  Kbd::pushChar('a');
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Char);
  CHECK(ke.ch == 'a');
}

TEST_CASE_FIXTURE(KbdFixture, "tryReadKey: char returned with correct casing")
{
  Kbd::pushChar('Z');
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.ch == 'Z');
}
