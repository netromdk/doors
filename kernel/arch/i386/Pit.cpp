#include <arch/i386/Pic.h>
#include <kernel/Io.h>
#include <kernel/Pit.h>

volatile uint64_t pitTicks{};

void Pit::init()
{
  Io::outb(PIT_CMD, PIT_CW);
  Io::outb(PIT_CH0, PIT_DIVISOR & 0xFF);
  Io::outb(PIT_CH0, PIT_DIVISOR >> 8);
  Pic::setMask(IRQ_TIMER, true);
}

void Pit::tick()
{
  uint64_t t = pitTicks;
  pitTicks = t + 1;
}

uint64_t Pit::uptimeMs()
{
  return pitTicks;
}

uint64_t Pit::uptimeSec()
{
  return pitTicks / 1000;
}

uint64_t Pit::msSince(uint64_t last)
{
  return pitTicks - last;
}
