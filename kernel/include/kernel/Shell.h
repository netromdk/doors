#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

#include <array>
#include <cstddef>
#include <span>
#include <string>

struct Command {
  string name;
  string desc;
  void (*handler)(int argc, const string *argv);
};

class Shell {
public:
  static void run();
  static int tokenize(const string &line, span<string> argv);
  static bool dispatch(span<const string> argv);
  static void registerCmd(const Command &cmd);
  static void printHelp();

private:
  static constexpr int MAX_CMDS = 32;
  static array<Command, MAX_CMDS> cmdTable;
  static int numCmds;

#ifndef __IS_DOORS_KERNEL
  friend struct ShellTestAccess;
#endif
};

#endif // KERNEL_SHELL_H
