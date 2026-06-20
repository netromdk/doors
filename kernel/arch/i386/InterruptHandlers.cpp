/**
 * These interrupt handlers will be called from isr.s if any are triggered.
 */

#ifndef KERNEL_I386_INTERRUPT_HANDLERS_H
#define KERNEL_I386_INTERRUPT_HANDLERS_H

#include <stdio.h>

#include <arch/i386/Pic.h>
#include <kernel/Kbd.h>
#include <kernel/Pit.h>

extern "C" {

void intDummy()
{
  printf("Dummy interrupt handler called. Find out which one!\n");
  Pic::sendEoi();
}

void intTick()
{
  Pit::tick();
  Pic::sendEoi();
}

void intKbd()
{
  Kbd::isrHandler();
  Pic::sendEoi();
}

} // extern "C"

#endif // KERNEL_I386_INTERRUPT_HANDLERS_H
