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

TEST_CASE("ctrl_a_goes_to_start")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "Xabc") == 0);
}

TEST_CASE("ctrl_e_goes_to_end")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar(Kbd::KEY_CTRL_E);
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abcX") == 0);
}

TEST_CASE("ctrl_b_moves_left")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between b and c
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abXc") == 0);
}

TEST_CASE("ctrl_f_moves_right")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar(Kbd::KEY_CTRL_F); // cursor after 'a'
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "aXbc") == 0);
}

TEST_CASE("ctrl_d_deletes_under_cursor")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar(Kbd::KEY_CTRL_D); // delete 'd'
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abce") == 0);
}

TEST_CASE("ctrl_k_kills_to_end")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar(Kbd::KEY_CTRL_K); // delete "de"
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abc") == 0);
}

TEST_CASE("backspace_in_middle")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar('\b');            // delete 'c'
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abde") == 0);
}

TEST_CASE("insert_in_middle")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('a');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between a and c
  Kbd::pushChar('b');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "abc") == 0);
}

TEST_CASE("ctrl_c_cancels")
{
  Kbd::init();
  char buf[64] = {};
  Kbd::pushChar('h');
  Kbd::pushChar('e');
  Kbd::pushChar('l');
  Kbd::pushChar(Kbd::KEY_CTRL_C);
  Kbd::pushChar('x');
  Kbd::pushChar('\n');
  Kbd::readLine(buf, sizeof(buf));

  // Ctrl+C returns immediately with empty buffer.
  // So subsequent chars remain in the ring buffer for the NEXT `readLine` call.
  CHECK(strcmp(buf, "") == 0);

  // Second call picks up the leftovers.
  Kbd::readLine(buf, sizeof(buf));
  CHECK(strcmp(buf, "x") == 0);
}
