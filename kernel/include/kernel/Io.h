#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <stdint.h>

class Io {
public:
  static uint8_t inb(uint16_t port);
  static void outb(uint16_t port, uint8_t value);
};

#endif // KERNEL_IO_H
