#include <doctest/doctest.h>
#include <kernel/Kbd.h>

TEST_CASE("tryReadKey: Unknown when buffer empty and no pending arrows")
{
  Kbd::init();
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Unknown);
  CHECK(ke.ch == 0);
}

TEST_CASE("tryReadKey: Up from pendingUp_ counter")
{
  Kbd::init();
  Kbd::pendingUp_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Up);
}

TEST_CASE("tryReadKey: Down from pendingDown_ counter")
{
  Kbd::init();
  Kbd::pendingDown_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Down);
}

TEST_CASE("tryReadKey: Left from pendingLeft_ counter")
{
  Kbd::init();
  Kbd::pendingLeft_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Left);
}

TEST_CASE("tryReadKey: Right from pendingRight_ counter")
{
  Kbd::init();
  Kbd::pendingRight_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Right);
}

TEST_CASE("tryReadKey: PageUp from pendingPageUp_ counter")
{
  Kbd::init();
  Kbd::pendingPageUp_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::PageUp);
}

TEST_CASE("tryReadKey: PageDown from pendingPageDown_ counter")
{
  Kbd::init();
  Kbd::pendingPageDown_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::PageDown);
}

TEST_CASE("tryReadKey: Home from pendingHome_ counter")
{
  Kbd::init();
  Kbd::pendingHome_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Home);
}

TEST_CASE("tryReadKey: End from pendingEnd_ counter")
{
  Kbd::init();
  Kbd::pendingEnd_ = 1;
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::End);
}

TEST_CASE("tryReadKey: drains exactly one counter per call")
{
  Kbd::init();
  Kbd::pendingUp_ = 3;
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Up);
  CHECK(Kbd::tryReadKey().key == Kbd::Key::Unknown);
}

TEST_CASE("tryReadKey: Up has priority over char buffer")
{
  Kbd::init();
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('x');

  auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Up);

  // Char should be available on next call.
  ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Char);
  CHECK(ke.ch == 'x');
}

TEST_CASE("tryReadKey: char returned")
{
  Kbd::init();
  Kbd::pushChar('a');
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.key == Kbd::Key::Char);
  CHECK(ke.ch == 'a');
}

TEST_CASE("tryReadKey: char returned with correct casing")
{
  Kbd::init();
  Kbd::pushChar('Z');
  const auto ke = Kbd::tryReadKey();
  CHECK(ke.ch == 'Z');
}
