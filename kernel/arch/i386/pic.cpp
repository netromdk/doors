#include <kernel/io.h>
#include <arch/i386/pic.h>

void Pic::init() {
  // ICW1: Start initialization in cascade mode.
  Io::outb(PIC1_CMD, ICW1_INIT + ICW1_ICW4);
  Io::outb(PIC2_CMD, ICW1_INIT + ICW1_ICW4);

  // ICW2: Set offsets.
  Io::outb(PIC1_DATA, 0x20); // Starts at 32.
  Io::outb(PIC2_DATA, 0x80); // Starts at 128.

  // ICW3.
  Io::outb(PIC1_DATA, 0x4); // Tell Master PIC that there is a slave PIC at IRQ2.
  Io::outb(PIC2_DATA, 0x2); // Tell Slave PIC its cascade identity.

  // ICW4.
  Io::outb(PIC1_DATA, ICW4_8086);
  Io::outb(PIC2_DATA, ICW4_8086);

  // Mask interrupts.
  Io::outb(PIC1_DATA, 0x0);
  Io::outb(PIC2_DATA, 0x0);
}
