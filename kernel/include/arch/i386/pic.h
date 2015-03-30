#ifndef ARCH_I386_PIC_H
#define ARCH_I386_PIC_H

/**
 * Inspiration found here:
 * - http://www.brokenthorn.com/Resources/OSDev16.html
 * - http://wiki.osdev.org/PIC#Programming_with_the_8259_PIC
 */

// Ports.
#define PIC1            0x20 // Master PIC.
#define PIC2            0xA0 // Slave PIC.
#define PIC1_CMD        PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_CMD        PIC2
#define PIC2_DATA       (PIC2+1)

// Initialization Command Words (ICW).
#define ICW1_ICW4       0x01 // Expect ICW4 bit.
#define ICW1_SINGLE     0x02 // Single (cascade) mode.
#define ICW1_INTERVAL4  0x04 // Call address interval 4 else 8.
#define ICW1_LEVEL      0x08 // Level triggered/edge mode.
#define ICW1_INIT       0x10 // Initialization - required!.

#define ICW2_L1         0x01 // Level 1 interrupt level.
#define ICW2_L2         0x02 // Level 2 interrupt level.
#define ICW2_L3         0x04 // Level 3 interrupt level.
#define ICW2_EOI        0x20 // End of Interrupt command.
#define ICW2_SL         0x40 // Select command.
#define ICW2_ROT        0x80 // Rotation command.

#define ICW3_RIS        0x01 // TODO: Find info on this.
#define ICW3_RIR        0x02 // TODO: Find info on this.
#define ICW3_MODE       0x04 // TODO: Find info on this.
#define ICW3_SMM        0x20 // TODO: Find info on this.
#define ICW3_ESMM       0x40 // TODO: Find info on this.
#define ICW3_D7         0x80 // TODO: Find info on this.

#define ICW4_8086       0x01 // 8086/88 (MCS-80/85) mode.
#define ICW4_AUTO       0x02 // Auto (normal) EOI.
#define ICW4_BUF_SLAVE  0x08 // Buffered slave mode.
#define ICW4_BUF_MASTER 0x0C // Buffered master mode.
#define ICW4_SFNM       0x10 // Special fully nested mode.

class Pic {
public:
  static void init();
};

#endif // ARCH_I386_PIC_H
