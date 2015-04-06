/**
 * These interrupt handlers will be called from isr.s if any are
 * triggered.
 */

#ifndef KERNEL_I386_INTERRUPT_HANDLERS_H
#define KERNEL_I386_INTERRUPT_HANDLERS_H

#include <kernel/kbd.h>
#include <arch/i386/pic.h>

extern "C" {
  void intDummy() {
    Pic::sendEoi();
  }

  void intTick() {
    /*
      static uint32_t ticks = 0;
      ticks++;
      printf("ticks: %u\n", ticks);
    */
    Pic::sendEoi();
  }

  void intKbd() {
    Kbd::readScanCode();
    Pic::sendEoi();
  }
}

#endif // KERNEL_I386_INTERRUPT_HANDLERS_H
