#ifndef TESTS_KBD_KBDFIXTURE_H
#define TESTS_KBD_KBDFIXTURE_H

#include <doctest/doctest.h>
#include <kernel/Kbd.h>

struct KbdFixture {
  KbdFixture()
  {
    Kbd::init();
  }
};

#endif // TESTS_KBD_KBDFIXTURE_H
