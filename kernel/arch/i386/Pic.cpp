#include <kernel/Io.h>
#include <arch/i386/Pic.h>

void Pic::init() {
  // Save masks
  uint8_t mask1 = Io::inb(PIC1_DATA),
    mask2 = Io::inb(PIC2_DATA);

  // ICW1: Start initialization in cascade mode.
  Io::outb(PIC1_CMD, ICW1_INIT + ICW1_ICW4);
  Io::outb(PIC2_CMD, ICW1_INIT + ICW1_ICW4);

  // ICW2: Set offsets.
  Io::outb(PIC1_DATA, 0x20); // Starts at 32.
  Io::outb(PIC2_DATA, 0x28); // Starts at 40.

  // ICW3.
  Io::outb(PIC1_DATA, 0x4); // Tell Master PIC that there is a slave PIC at IRQ2.
  Io::outb(PIC2_DATA, 0x2); // Tell Slave PIC its cascade identity.

  // ICW4.
  Io::outb(PIC1_DATA, ICW4_8086);
  Io::outb(PIC2_DATA, ICW4_8086);

  // Restore masks.
  Io::outb(PIC1_DATA, mask1);
  Io::outb(PIC2_DATA, mask2);
}

void Pic::enableInt() {
  // Set interrupt-enable flag to enable interrupts.
  __asm__("sti");
}

void Pic::disableInt() {
  // Clear interrupt-enable flag to disable interrupts.
  __asm__("cli");
}

bool Pic::isIntEnabled() {
  uint32_t flags;
  __asm__("pushf;"
          "pop %0;"
          : "=g" (flags));
  return flags & (1 << 9);
}

void Pic::sendEoi() {
  // Send to both master and slave PIC.
  Io::outb(PIC1, ICW2_EOI);
  Io::outb(PIC2, ICW2_EOI);
}

void Pic::setMask(uint8_t mask, bool clear) {
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

uint16_t Pic::getIsr() {
  return getReg(OCW3_RIS);
}

uint16_t Pic::getIrr() {
  return getReg(OCW3_RIR);
}

uint16_t Pic::getReg(uint8_t cmd) {
  Io::outb(PIC1_CMD, cmd);
  Io::outb(PIC2_CMD, cmd);
  return (Io::inb(PIC2_CMD) << 8) | Io::inb(PIC1_CMD);
}
