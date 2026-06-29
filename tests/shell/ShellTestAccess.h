#ifndef TESTS_SHELL_SHELLTESTACCESS_H
#define TESTS_SHELL_SHELLTESTACCESS_H

#include <kernel/Shell.h>

struct ShellTestAccess {
  static void resetCommands()
  {
    for (int i = 0; i < Shell::numCmds; i++) {
      Shell::cmdTable[i].name.clear();
      Shell::cmdTable[i].desc.clear();
    }
    Shell::numCmds = 0;
  }
};

#endif // TESTS_SHELL_SHELLTESTACCESS_H
