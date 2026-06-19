#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <kernel/Kbd.h>
#include <kernel/Shell.h>
#include <arch/i386/Pic.h>

namespace {

constexpr int MAX_CMDS = 32;
Command cmdTable[MAX_CMDS];
int numCmds = 0;

} // namespace

void Shell::run()
{
  Pic::enableInt();
  char line[256];
  char *argv[16];
  constexpr int MAX_ARGS = sizeof(argv) / sizeof(*argv);
  for (;;) {
    printf("> ");
    Kbd::readLine(line, sizeof(line));
    int argc = Shell::tokenize(line, argv, MAX_ARGS);
    Shell::dispatch(argc, argv);
  }
}

int Shell::tokenize(char *line, char **argv, int max)
{
  int argc = 0;
  char *p = line;
  while (*p) {
    while (isspace(*p)) {
      p++;
    }
    if (!*p) {
      break;
    }
    if (argc >= max - 1) {
      break;
    }
    argv[argc++] = p;
    while (*p && !isspace(*p)) {
      p++;
    }
    if (*p) {
      *p++ = '\0';
    }
  }
  argv[argc] = nullptr;
  return argc;
}

bool Shell::dispatch(int argc, char **argv)
{
  if (argc == 0) {
    return true;
  }

  for (int i = 0; i < numCmds; i++) {
    if (strcmp(argv[0], cmdTable[i].name) == 0) {
      cmdTable[i].handler(argc, argv);
      return true;
    }
  }

  printf("Unknown command: %s\n", argv[0]);
  return false;
}

void Shell::printHelp()
{
  printf("Commands:\n");
  for (int i = 0; i < numCmds; i++) {
    if (cmdTable[i].desc) {
      printf("  %s - %s\n", cmdTable[i].name, cmdTable[i].desc);
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
