#include <arch/i386/Pic.h>
#include <kernel/Io.h>

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
