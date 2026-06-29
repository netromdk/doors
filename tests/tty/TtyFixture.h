#ifndef TESTS_TTY_TTYFIXTURE_H
#define TESTS_TTY_TTYFIXTURE_H

#include "TtyTestAccess.h"
#include <doctest/doctest.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

struct TtyFixture {
  TtyFixture()
  {
    TtyTestAccess::reset();
  }
};

#endif // TESTS_TTY_TTYFIXTURE_H
