#ifndef TESTS_SHELL_TESTHELPERS_H
#define TESTS_SHELL_TESTHELPERS_H

#include <kernel/Shell.h>

template <size_t N>
bool dispatchLine(const string &line, string (&argv)[N])
{
  int argc = Shell::tokenize(line, argv, N);
  return Shell::dispatch(argc, argv);
}

#endif // TESTS_SHELL_TESTHELPERS_H
