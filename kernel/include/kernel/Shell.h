#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

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
};

#endif // KERNEL_SHELL_H
