#include <kernel/Io.h>

uint8_t Io::inb(uint16_t port) {
  uint8_t b;
  __asm__("inb %%dx, %%al"
          : "=a" (b)
          : "d" (port));
  return b;
}

void Io::outb(uint16_t port, uint8_t value) {
  __asm__("outb %%al, %%dx"
          :
          : "d" (port),
            "a" (value));
}
