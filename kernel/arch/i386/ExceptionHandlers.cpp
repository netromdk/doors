/**
 * These exception handlers will be called from isr.s if any are
 * triggered.
 */

#ifndef KERNEL_I386_EXCEPTION_HANDLERS_H
#define KERNEL_I386_EXCEPTION_HANDLERS_H

#include <stdio.h>
#include <stdlib.h>

extern "C" {
  void excDivZero() {
    printf("Divide by zero!\n");
    abort();
  }

  void excInvOp() {
    printf("Invalid opcode!\n");
    abort();
  }

  void excSegNp() {
    printf("Segment not present!\n");
    abort();
  }

  void excSf() {
    printf("Stack fault!\n");
    abort();
  }

  void excGp() {
    printf("General protection exception!\n");
    abort();
  }

  void excPf() {
    printf("Page fault!\n");
    abort();
  }
}

#endif // KERNEL_I386_EXCEPTION_HANDLERS_H
