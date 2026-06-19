#ifndef KERNEL_SHELL_H
#define KERNEL_SHELL_H

#include <stddef.h>

struct Command {
  const char *name;
  const char *desc;
  void (*handler)(int argc, char **argv);
};

class Shell {
public:
  static void run();
  static int tokenize(char *line, char **argv, int max);
  static bool dispatch(int argc, char **argv);
  static void registerCmd(const Command &cmd);
};

#endif // KERNEL_SHELL_H
