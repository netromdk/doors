#ifndef ARCH_I386_PIC_H
#define ARCH_I386_PIC_H

/**
 * Inspiration found here:
 * - http://www.brokenthorn.com/Resources/OSDev16.html
 * - http://wiki.osdev.org/PIC#Programming_with_the_8259_PIC
 */

#include <stdint.h>

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

// Output Command Words (OCW).
#define OCW3_RIS        0x0A // Read In-Service Register.
#define OCW3_RIR        0x0B // Read Interrupt Request Register.

/* Hardware interrupts */

// PIC1 / Master
#define IRQ0         0x08
#define IRQ1         0x09
#define IRQ2         0x0A
#define IRQ3         0x0B
#define IRQ4         0x0C
#define IRQ5         0x0D
#define IRQ6         0x0E
#define IRQ7         0x0F

#define IRQ_TIMER    IRQ0
#define IRQ_KEYBOARD IRQ1
#define IRQ_CASCADE  IRQ2
#define IRQ_SERIAL2  IRQ3
#define IRQ_SERIAL1  IRQ4
#define IRQ_PPORT2   IRQ5 // At systems: parallel port 2, PS/2 systems: reserved.
#define IRQ_DISKETTE IRQ6
#define IRQ_PPORT1   IRQ7

// PIC2 / Slave
#define IRQ8         0x70
#define IRQ9         0x71
#define IRQ10        0x72
#define IRQ11        0x73
#define IRQ12        0x74
#define IRQ13        0x75
#define IRQ14        0x76
#define IRQ15        0x77

#define IRQ_RTC      IRQ8  // CMOS real-time clock
#define IRQ_CGAVR    IRQ9  // CGA vertical retrace.
#define IRQ_AUX      IRQ12 // At systems: reserved, PS/2 systems: auxiliary device.
#define IRQ_FPU      IRQ13
#define IRQ_HDDCTRL  IRQ14 // Hard disk controller.

class Pic {
public:
  static void init();

  static void enableInt();
  static void disableInt();
  static bool isIntEnabled();

  /**
   * Acknowledge interrupt to PIC (by sending end-of-interrupt) so it
   * will process new/pending ones.
   */
  static void sendEoi();

  /**
   * Manipulates the PIC's Interrupt Mask Register (IMR) which is 8
   * bytes wide.
   */
  static void setMask(uint8_t mask, bool clear = false);

  /**
   * Read the In-Service Register (cascaded value from both PICs).
   */
  static uint16_t getIsr();

  /**
   * Read the Interrupt Requesrt Register (cascaded value from both
   * PICs).
   */
  static uint16_t getIrr();

private:
  /**
   * Reads the cascaded value from IRR or ISR from both PICs.
   */
  static uint16_t getReg(uint8_t cmd);
};

#endif // ARCH_I386_PIC_H
