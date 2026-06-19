#include <arch/i386/Pic.h>
#include <kernel/Io.h>
#include <kernel/Tty.h>

uint8_t Io::inb(uint16_t)
{
  return 0;
}

void Io::outb(uint16_t, uint8_t)
{
}

void Pic::setMask(uint8_t, bool)
{
}

void Pic::enableInt()
{
}

void Pic::disableInt()
{
}

// Weak stubs so test targets, that also link the real Tty.cpp (test_tty), pick up the real
// implementations instead. But otherwise they are stubs.
__attribute__((weak)) void Tty::cursorEnable()
{
}
__attribute__((weak)) void Tty::cursorDisable()
{
}
__attribute__((weak)) void Tty::cursorSetPos(uint8_t, uint8_t)
{
}
