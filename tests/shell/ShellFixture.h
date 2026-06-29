#ifndef TESTS_SHELL_SHELLFIXTURE_H
#define TESTS_SHELL_SHELLFIXTURE_H

#include <doctest/doctest.h>

#include <kernel/Kbd.h>

#include "ShellTestAccess.h"

struct ShellFixture {
  ShellFixture()
  {
    Kbd::init();
    ShellTestAccess::resetCommands();
  }
};

#endif // TESTS_SHELL_SHELLFIXTURE_H
