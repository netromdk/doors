#include <kernel/io.h>
#include <arch/i386/pic.h>

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
