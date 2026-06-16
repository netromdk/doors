#ifndef ARCH_I386_PIC_H
#define ARCH_I386_PIC_H

/**
 * Inspiration found here:
 * - http://www.brokenthorn.com/Resources/OSDev16.html
 * - http://wiki.osdev.org/PIC#Programming_with_the_8259_PIC
 */

#include <stdint.h>

// Ports.
static constexpr uint16_t PIC1 = 0x20, // Master PIC.
  PIC2 = 0xA0,                         // Slave PIC.
  PIC1_CMD = PIC1,
  PIC1_DATA = (PIC1 + 1),
  PIC2_CMD = PIC2,
  PIC2_DATA = (PIC2 + 1);

// Initialization Command Words (ICW).
static constexpr uint8_t ICW1_ICW4 = 0x01, // Expect ICW4 bit.
  ICW1_SINGLE = 0x02,                      // Single (cascade) mode.
  ICW1_INTERVAL4 = 0x04,                   // Call address interval 4 else 8.
  ICW1_LEVEL = 0x08,                       // Level triggered/edge mode.
  ICW1_INIT = 0x10,                        // Initialization - required!.

  ICW2_L1 = 0x01,  // Level 1 interrupt level.
  ICW2_L2 = 0x02,  // Level 2 interrupt level.
  ICW2_L3 = 0x04,  // Level 3 interrupt level.
  ICW2_EOI = 0x20, // End of Interrupt command.
  ICW2_SL = 0x40,  // Select command.
  ICW2_ROT = 0x80, // Rotation command.

  ICW3_RIS = 0x01,  // TODO: Find info on this.
  ICW3_RIR = 0x02,  // TODO: Find info on this.
  ICW3_MODE = 0x04, // TODO: Find info on this.
  ICW3_SMM = 0x20,  // TODO: Find info on this.
  ICW3_ESMM = 0x40, // TODO: Find info on this.
  ICW3_D7 = 0x80,   // TODO: Find info on this.

  ICW4_8086 = 0x01,       // 8086/88 (MCS-80/85) mode.
  ICW4_AUTO = 0x02,       // Auto (normal) EOI.
  ICW4_BUF_SLAVE = 0x08,  // Buffered slave mode.
  ICW4_BUF_MASTER = 0x0C, // Buffered master mode.
  ICW4_SFNM = 0x10;       // Special fully nested mode.

// Output Command Words (OCW).
static constexpr uint8_t OCW3_RIS = 0x0A, // Read In-Service Register.
  OCW3_RIR = 0x0B;                        // Read Interrupt Request Register.

/* Hardware interrupts */

// PIC1 / Master
static constexpr uint8_t IRQ0 = 0x08,
  IRQ1 = 0x09,
  IRQ2 = 0x0A,
  IRQ3 = 0x0B,
  IRQ4 = 0x0C,
  IRQ5 = 0x0D,
  IRQ6 = 0x0E,
  IRQ7 = 0x0F,

  IRQ_TIMER = IRQ0,
  IRQ_KEYBOARD = IRQ1,
  IRQ_CASCADE = IRQ2,
  IRQ_SERIAL2 = IRQ3,
  IRQ_SERIAL1 = IRQ4,
  IRQ_PPORT2 = IRQ5, // At systems: parallel port 2, PS/2 systems: reserved.
  IRQ_DISKETTE = IRQ6,
  IRQ_PPORT1 = IRQ7,

  // PIC2 / Slave
  IRQ8 = 0x70,
  IRQ9 = 0x71,
  IRQ10 = 0x72,
  IRQ11 = 0x73,
  IRQ12 = 0x74,
  IRQ13 = 0x75,
  IRQ14 = 0x76,
  IRQ15 = 0x77,

  IRQ_RTC = IRQ8,      // CMOS real-time clock
  IRQ_CGAVR = IRQ9,    // CGA vertical retrace.
  IRQ_AUX = IRQ12,     // At systems: reserved, PS/2 systems: auxiliary device.
  IRQ_FPU = IRQ13,
  IRQ_HDDCTRL = IRQ14; // Hard disk controller.

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
