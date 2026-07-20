#include <cstdint>
#include <kernel/Cpu.h>

namespace {

// Simulated EFLAGS register. Bit 9 (IF) = 1 by default. Bit 1 set.
uint32_t trackedEflags = 0x202;
int disableCount = 0;
int enableCount = 0;

} // namespace

void Cpu::disableInterrupts()
{
  ++disableCount;
  trackedEflags &= ~(1u << 9);
}

void Cpu::enableInterrupts()
{
  ++enableCount;
  trackedEflags |= (1u << 9);
}

bool Cpu::interruptsEnabled()
{
  return (trackedEflags & (1u << 9)) != 0;
}

uint32_t Cpu::getEflags()
{
  return trackedEflags;
}

void Cpu::setEflags(uint32_t eflags)
{
  trackedEflags = eflags;
}

void cpuTestReset()
{
  trackedEflags = 0x202;
  disableCount = 0;
  enableCount = 0;
}

int cpuTestDisableCount()
{
  return disableCount;
}

int cpuTestEnableCount()
{
  return enableCount;
}
