#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <cstdint>

class Io {
public:
  static uint8_t inb(uint16_t port);
  static void outb(uint16_t port, uint8_t value);
  static void outw(uint16_t port, uint16_t value);
  static void outl(uint16_t port, uint32_t value);
};

#endif // KERNEL_IO_H
