#include <kernel/Io.h>
#include <stdint.h>

uint8_t Io::inb(uint16_t)
{
  return 0;
}

void Io::outb(uint16_t, uint8_t)
{
}
