#include <cstdio>
#include <cstring>
#include <string>

#include <kernel/Kbd.h>

#include "KbdFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr int HIST_SIZE = 8;

struct HistoryFixture {
  string histBuf[HIST_SIZE];
  int pos{-1};
  HistoryCtx ctx{};

  HistoryFixture()
  {
    ctx.buf = histBuf;
    ctx.size = HIST_SIZE;
    ctx.count = 0;
    ctx.head = 0;
    ctx.pos = &pos;
  }

  void addEntry(const char *text)
  {
    histBuf[ctx.head] = text;
    ctx.head = (ctx.head + 1) % HIST_SIZE;
    if (ctx.count < HIST_SIZE) {
      ++ctx.count;
    }
  }
};

} // namespace

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
  for (char c : buf) {
    CHECK(c == 'x');
  }
}

TEST_CASE_FIXTURE(KbdFixture, "whitespace")
{
  string buf;
  string input = "  hello world  \n";
  for (char c : input) {
    Kbd::pushChar(c);
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

TEST_CASE_FIXTURE(KbdFixture, "history: up loads most recent entry")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");
  hf.addEntry("third");

  string buf;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "third");
}

TEST_CASE_FIXTURE(KbdFixture, "history: up twice loads second-most-recent")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");
  hf.addEntry("third");

  string buf;
  Kbd::pendingUp_ = 2;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "second");
}

TEST_CASE_FIXTURE(KbdFixture, "history: up three loads oldest entry")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");
  hf.addEntry("third");

  string buf;
  Kbd::pendingUp_ = 3;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "first");
}

TEST_CASE_FIXTURE(KbdFixture, "history: up four stays at oldest entry")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");
  hf.addEntry("third");

  string buf;
  Kbd::pendingUp_ = 4;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "first");
}

TEST_CASE_FIXTURE(KbdFixture, "history: down after up clears line")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");

  string buf;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "second");

  Kbd::pendingUp_ = 1;
  Kbd::pendingDown_ = 1;
  Kbd::pushChar('\n');
  buf.clear();
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "history: empty history is no-op")
{
  HistoryFixture hf;

  string buf;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "history: down when not browsing is no-op")
{
  HistoryFixture hf;
  hf.addEntry("first");

  string buf;
  Kbd::pendingDown_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "history: ctrl_p triggers history up")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");

  string buf;
  Kbd::pushChar(Kbd::KEY_CTRL_P);
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "second");
}

TEST_CASE_FIXTURE(KbdFixture, "history: ctrl_n triggers history down")
{
  HistoryFixture hf;
  hf.addEntry("first");
  hf.addEntry("second");

  string buf;
  Kbd::pushChar(Kbd::KEY_CTRL_P);
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "second");

  Kbd::pushChar(Kbd::KEY_CTRL_N);
  buf.clear();
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "history: circular buffer wrap-around")
{
  HistoryFixture hf;
  for (int i = 0; i < HIST_SIZE + 2; ++i) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "entry%d", i);
    hf.addEntry(tmp);
  }

  string buf;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "entry9");

  Kbd::pendingUp_ = 1;
  buf.clear();
  Kbd::pushChar('\n');
  Kbd::readLine(buf, &hf.ctx);
  CHECK(buf == "entry8");
}

TEST_CASE_FIXTURE(KbdFixture, "navigation: pending_up with no history is no-op")
{
  string buf;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "navigation: pending_down with no history is no-op")
{
  string buf;
  Kbd::pendingDown_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "");
}

TEST_CASE_FIXTURE(KbdFixture, "navigation: pending_left_at_start")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pushChar('b');
  Kbd::pendingLeft_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "ab");
}

TEST_CASE_FIXTURE(KbdFixture, "navigation: pending_right_at_end")
{
  string buf;
  Kbd::pushChar('a');
  Kbd::pendingRight_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(buf);
  CHECK(buf == "a");
}
