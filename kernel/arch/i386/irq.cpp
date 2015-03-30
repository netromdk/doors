#include <stdint.h>

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
