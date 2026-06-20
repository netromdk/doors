#include <doctest/doctest.h>
#include <stdint.h>
#include <string.h>

#include <kernel/Kbd.h>

namespace {

constexpr int HISTORY_SIZE = 3;
constexpr int LINE_SIZE = 256;

} // namespace

TEST_CASE("arrow_up_loads_most_recent")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "cmd2") == 0);
}

TEST_CASE("arrow_up_twice_goes_older")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingUp_ = 2;
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "cmd1") == 0);
}

TEST_CASE("arrow_down_at_end_stays_empty")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingDown_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "") == 0);
}

TEST_CASE("arrow_down_after_up_returns_to_empty")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};

  // Up twice (cmd2 -> cmd1), then down once (back to empty).
  Kbd::pendingUp_ = 2;
  Kbd::pendingDown_ = 1;

  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "") == 0);
}

TEST_CASE("ctrLP_loads_most_recent")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pushChar(Kbd::KEY_CTRL_P);
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "cmd2") == 0);
}

TEST_CASE("ctrLN_after_up_goes_forward")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "cmd0");
  strcpy(hbuf[1], "cmd1");
  strcpy(hbuf[2], "cmd2");
  int count = 3;
  int head = 3;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingUp_ = 1; // -> cmd2
  Kbd::pushChar(Kbd::KEY_CTRL_N);
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "") == 0);
}

TEST_CASE("arrow_up_empty_history")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  int count = 0;
  int head = 0;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "") == 0);
}

TEST_CASE("history_arrow_echoes_command")
{
  char hbuf[HISTORY_SIZE][LINE_SIZE]{};
  strcpy(hbuf[0], "hello");
  int count = 1;
  int head = 1;
  int pos = -1;
  HistoryCtx hctx{hbuf, HISTORY_SIZE, count, head, &pos};

  char line[LINE_SIZE]{};
  Kbd::pendingUp_ = 1;
  Kbd::pushChar('\n');
  Kbd::readLine(line, sizeof(line), &hctx);

  CHECK(strcmp(line, "hello") == 0);
}
