#include <array>
#include <cctype>
#include <cstdio>

#include <arch/i386/Pic.h>
#include <kernel/Kbd.h>
#include <kernel/Shell.h>
#include <kernel/Tty.h>

namespace {

constexpr int MAX_CMDS = 32;
array<Command, MAX_CMDS> cmdTable;
int numCmds = 0;

// Command history ring buffer.
constexpr int HISTORY_MAX = 100;
static array<string, HISTORY_MAX> historyBuf_;
static constinit int historyCount_ = 0;
static constinit int historyHead_ = 0;

} // namespace

void Shell::run()
{
  Pic::enableInt();
  Tty::cursorEnable();
  string line;
  array<string, 16> argv;
  constexpr int MAX_ARGS = 16;
  int historyPos = -1;

  for (;;) {
    HistoryCtx hctx{historyBuf_.data(), HISTORY_MAX, historyCount_, historyHead_, &historyPos};

    Tty::setColor(vgaColor(COLOR_LIGHT_GREY, COLOR_BLACK));
    printf("> ");
    Tty::setColor(Tty::DEFAULT_COLOR);
    Kbd::readLine(line, &hctx);

    const int argc = Shell::tokenize(line, argv.data(), MAX_ARGS);
    Shell::dispatch(argc, argv.data());

    // Save to history but skip empty or duplicate of the most recent entry.
    if (!line.empty() && (historyCount_ == 0 ||
                          line != historyBuf_[(historyHead_ - 1 + HISTORY_MAX) % HISTORY_MAX])) {
      historyBuf_[historyHead_] = line;
      historyHead_ = (historyHead_ + 1) % HISTORY_MAX;
      if (historyCount_ < HISTORY_MAX) {
        historyCount_++;
      }
    }
    historyPos = -1;
  }
}

int Shell::tokenize(const string &line, string *argv, int max)
{
  int argc = 0;
  size_t i = 0;
  string::size_type n = line.size();
  while (i < n) {
    while (i < n && isspace(static_cast<unsigned char>(line[i]))) {
      i++;
    }
    if (i >= n) {
      break;
    }
    if (argc >= max - 1) {
      break;
    }
    string::size_type start = i;
    while (i < n && !isspace(static_cast<unsigned char>(line[i]))) {
      i++;
    }
    argv[argc] = line.substr(start, i - start);
    argc++;
  }
  argv[argc].clear();
  return argc;
}

bool Shell::dispatch(int argc, const string *argv)
{
  if (argc == 0) {
    return true;
  }

  for (int i = 0; i < numCmds; i++) {
    if (argv[0] == cmdTable[i].name) {
      cmdTable[i].handler(argc, argv);
      return true;
    }
  }

  printf("Unknown command: %s\n", argv[0].c_str());
  return false;
}

void Shell::printHelp()
{
  printf("Commands:\n");
  for (int i = 0; i < numCmds; i++) {
    if (!cmdTable[i].desc.empty()) {
      printf("  %s - %s\n", cmdTable[i].name.c_str(), cmdTable[i].desc.c_str());
    }
  }
}

void Shell::registerCmd(const Command &cmd)
{
  if (numCmds >= MAX_CMDS) {
    return;
  }
  cmdTable[numCmds++] = cmd;
}
