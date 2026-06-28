#ifndef TESTS_SHELL_TESTHELPERS_H
#define TESTS_SHELL_TESTHELPERS_H

#include <kernel/Shell.h>

template <size_t N>
bool dispatchLine(const string &line, string (&argv)[N])
{
  const int argc = Shell::tokenize(line, {argv, N});
  return Shell::dispatch({argv, static_cast<size_t>(argc)});
}

#endif // TESTS_SHELL_TESTHELPERS_H
