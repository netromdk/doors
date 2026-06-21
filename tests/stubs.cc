#include <arch/i386/Pic.h>
#include <kernel/Io.h>
#include <kernel/Kbd.h>
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

// Weak stubs so test targets, that also link the real kernel source code (like Tty.cc and target
// test_tty), pick up the real implementations instead. But otherwise they are stubs.

__attribute__((weak)) void Tty::cursorEnable()
{
}

__attribute__((weak)) void Tty::cursorDisable()
{
}

__attribute__((weak)) void Tty::cursorSetPos(uint8_t, uint8_t)
{
}

__attribute__((weak)) int Tty::scrollbackSize()
{
  return 0;
}

__attribute__((weak)) const char *Tty::scrollbackLine(int)
{
  return nullptr;
}

__attribute__((weak)) void Tty::scrollbackShow(int)
{
}

__attribute__((weak)) void Tty::scrollbackExit()
{
}

__attribute__((weak)) bool Tty::scrollbackActive()
{
  return false;
}

__attribute__((weak)) void Tty::scrollbackPageUp()
{
}

__attribute__((weak)) void Tty::scrollbackPageDown()
{
}

__attribute__((weak)) void Tty::scrollbackHome()
{
}

__attribute__((weak)) void Tty::scrollbackLineUp()
{
}

__attribute__((weak)) void Tty::scrollbackLineDown()
{
}

__attribute__((weak)) void Kbd::clearNavigation()
{
}
