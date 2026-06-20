#include <doctest/doctest.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>
#include <string.h>

TEST_CASE("simple")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('h');
  Kbd::pushChar('e');
  Kbd::pushChar('l');
  Kbd::pushChar('l');
  Kbd::pushChar('o');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "hello") == 0);
}

TEST_CASE("backspace")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('\b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "acd") == 0);
}

TEST_CASE("empty")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "") == 0);
}

TEST_CASE("max_chars")
{
  Kbd::init();
  char buf[32] = {};
  for (int i = 0; i < 100; i++) {
    Kbd::pushChar('x');
  }
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strlen(buf) == sizeof(buf) - 1); // 31 chars max (buf[31] is null)
  for (size_t i = 0; i < strlen(buf); i++) {
    CHECK(buf[i] == 'x');
  }
}

TEST_CASE("whitespace")
{
  Kbd::init();
  char buf[64] = {};
  char input[] = "  hello world  \n";
  for (char *p = input; *p; p++) {
    Kbd::pushChar(*p);
  }
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "  hello world  ") == 0);
}

TEST_CASE("ctrl_u")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_U);
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar('f');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "def") == 0);
}

TEST_CASE("multi_backspace")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('\b');
  Kbd::pushChar('\b');
  Kbd::pushChar('\b');
  Kbd::pushChar('\b');
  Kbd::pushChar('\b'); // One extra past empty
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "") == 0);
}
