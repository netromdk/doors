#include <doctest/doctest.h>
#include <stdint.h>
#include <string.h>

#include <kernel/Kbd.h>

namespace {

constexpr int HISTORY_SIZE = 3;

} // namespace

TEST_CASE("arrow_up_loads_most_recent")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "cmd2");
}

TEST_CASE("arrow_up_twice_goes_older")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingUp_ = 2;
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "cmd1");
}

TEST_CASE("arrow_down_at_end_stays_empty")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingDown_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "");
}

TEST_CASE("arrow_down_after_up_returns_to_empty")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;

  // Up twice (cmd2 -> cmd1), then down once (back to empty).
  Kbd::pendingUp_ = 2;
  Kbd::pendingDown_ = 1;

  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "");
}

TEST_CASE("ctrLP_loads_most_recent")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pushChar(Kbd::KEY_CTRL_P);
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "cmd2");
}

TEST_CASE("ctrLN_after_up_goes_forward")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "cmd0";
  hbuf[1] = "cmd1";
  hbuf[2] = "cmd2";
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingUp_ = 1; // -> cmd2
  Kbd::pushChar(Kbd::KEY_CTRL_N);
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "");
}

TEST_CASE("arrow_up_empty_history")
{
  string hbuf[HISTORY_SIZE];
  int count = 0;
  int head = 0;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "");
}

TEST_CASE("history_arrow_echoes_command")
{
  string hbuf[HISTORY_SIZE];
  hbuf[0] = "hello";
  int count = 1;
  int head = 1;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  string line;
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, &hctx);

  CHECK(line == "hello");
}
