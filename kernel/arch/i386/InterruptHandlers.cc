/**
 * These interrupt handlers will be called from isr.s if any are triggered.
 */

#include <cstdio>

#include <arch/i386/Pic.h>
#include <kernel/Kbd.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>

extern "C" {

void intDummy()
{
  printf("Dummy interrupt handler called. Find out which one!\n");
  Pic::sendEoi();
}

uint32_t intTick(uint32_t currentEsp)
{
  Pit::tick();
  const uint32_t nextEsp = Scheduler::tick(currentEsp);
  Pic::sendEoi();
  return nextEsp;
}

void intKbd()
{
  Kbd::isrHandler();
  Pic::sendEoi();
}

} // extern "C"
