#ifndef TESTS_TTY_TTYTESTACCESS_H
#define TESTS_TTY_TTYTESTACCESS_H

#include <algorithm>
#include <cstring>

#include <kernel/Semaphore.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

struct TtyTestAccess {
  static void reset()
  {
    Tty::lock_ = Semaphore(1);
    Tty::termRow_ = 0;
    Tty::termCol_ = 0;
    Tty::termColor_ = Tty::DEFAULT_COLOR;
    Tty::scrolling_ = true;
    Tty::scrollbackHead_ = 0;
    Tty::scrollbackCount_ = 0;
    Tty::scrollbackActive_ = false;
    Tty::scrollbackOffset_ = 0;
    for (size_t i = 0; i < size_t(VGA_WIDTH) * Tty::ROWS; i++) {
      VGA_RAM[i] = vgaEntry(' ', Tty::DEFAULT_COLOR);
    }
    for (size_t i = 0; i < size_t(VGA_WIDTH) * Tty::ROWS; i++) {
      reinterpret_cast<uint16_t *>(Tty::savedScreen_.data())[i] = 0;
    }
  }
};

#endif // TESTS_TTY_TTYTESTACCESS_H
