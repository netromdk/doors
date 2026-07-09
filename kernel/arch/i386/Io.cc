#include <kernel/Io.h>

uint8_t Io::inb(uint16_t port)
{
  uint8_t b;
  __asm__("inb %%dx, %%al" : "=a"(b) : "d"(port));
  return b;
}

void Io::outb(uint16_t port, uint8_t value)
{
  __asm__("outb %%al, %%dx" : : "d"(port), "a"(value));
}

void Io::outw(uint16_t port, uint16_t value)
{
  __asm__("outw %%ax, %%dx" : : "d"(port), "a"(value));
}

void Io::outl(uint16_t port, uint32_t value)
{
  __asm__("outl %%eax, %%dx" : : "d"(port), "a"(value));
}

void Io::signalShutdown(uint32_t code)
{
  // Port 0xf4 matches the `-device isa-debug-exit,iobase=0xf4` QEMU argument in
  // "cmake/run-all-tests.cmake" and "cmake/run-qemu-timeout.cmake".
  outl(0xf4, code);
}
