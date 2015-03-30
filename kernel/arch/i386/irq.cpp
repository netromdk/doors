#include <stdint.h>

#include <kernel/io.h>
#include <arch/i386/pic.h>
#include <arch/i386/irq.h>

void Irq::enable() {
  __asm__("sti");
}

void Irq::disable() {
  __asm__("cli");
}

bool Irq::isEnabled() {
  uint32_t flags;
  __asm__("pushf;"
          "pop %0;"
          : "=g" (flags));
  return flags & (1 << 9);
}

void Irq::setMask(uint8_t mask, bool clear) {
  uint16_t port;
  if (mask < 8) {
    port = PIC1_DATA;
  }
  else {
    port = PIC2_DATA;
    mask -= 8;
  }

  uint8_t value;
  if (!clear) {
    value = Io::inb(port) | (1 << mask);
  }
  else {
    value = Io::inb(port) & ~(1 << mask);
  }

  Io::outb(port, value);
}
