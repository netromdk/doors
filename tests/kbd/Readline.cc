#include "KbdFixture.h"
#include <doctest/doctest.h>
#include <kernel/Keymap.h>
#include <string.h>

TEST_CASE_FIXTURE(KbdFixture, "simple")
{
  string buf;
  Kbd::pushChar('h');
  Kbd::pushChar('e');
  Kbd::pushChar('l');
  Kbd::pushChar('l');
  Kbd::pushChar('o');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "hello");
}

TEST_CASE_FIXTURE(KbdFixture, "backspace")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('\b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "acd");
}

TEST_CASE_FIXTURE(KbdFixture, "empty")
{
  string buf;
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "many_chars")
{
  string buf;
  for (int i = 0; i < 100; i++) {
    Kbd::pushChar('x');
  }
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf.size() == 100);
  for (size_t i = 0; i < buf.size(); i++) {
    CHECK(buf[i] == 'x');
  }
}

TEST_CASE_FIXTURE(KbdFixture, "whitespace")
{
  string buf;
  string input = "  hello world  \n";
  for (size_t i = 0; i < input.size(); i++) {
    Kbd::pushChar(input[i]);
  }
  Kbd::readLine(buf);
  CHECK(buf == "  hello world  ");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_u")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_U);
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar('f');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "def");
}

TEST_CASE_FIXTURE(KbdFixture, "multi_backspace")
{
  string buf;
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
  Kbd::readLine(buf);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_a_goes_to_start")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "Xabc");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_e_goes_to_end")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar(Kbd::KEY_CTRL_E);
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abcX");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_b_moves_left")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between b and c
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abXc");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_f_moves_right")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_A);
  Kbd::pushChar(Kbd::KEY_CTRL_F); // cursor after 'a'
  Kbd::pushChar('X');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "aXbc");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_d_deletes_under_cursor")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar(Kbd::KEY_CTRL_D); // delete 'd'
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abce");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_k_kills_to_end")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar(Kbd::KEY_CTRL_K); // delete "de"
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abc");
}

TEST_CASE_FIXTURE(KbdFixture, "backspace_in_middle")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pushChar('c');
  Kbd::pushChar('d');
  Kbd::pushChar('e');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between d and e
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between c and d
  Kbd::pushChar('\b');            // delete 'c'
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abde");
}

TEST_CASE_FIXTURE(KbdFixture, "insert_in_middle")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('c');
  Kbd::pushChar(Kbd::KEY_CTRL_B); // cursor between a and c
  Kbd::pushChar('b');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "abc");
}

TEST_CASE_FIXTURE(KbdFixture, "ctrl_c_cancels")
{
  string buf;
  Kbd::pushChar('h');
  Kbd::pushChar('e');
  Kbd::pushChar('l');
  Kbd::pushChar(Kbd::KEY_CTRL_C);
  Kbd::pushChar('x');
  Kbd::pushChar('\n');
  Kbd::readLine(buf);

  // Ctrl+C returns immediately with empty buffer.
  // So subsequent chars remain in the ring buffer for the NEXT `readLine` call.
  CHECK(buf == "");

  // Second call picks up the leftovers.
  Kbd::readLine(buf);
  CHECK(buf == "x");
}
