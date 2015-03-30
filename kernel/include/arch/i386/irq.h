#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

// Hardware interrupts.
#define IRQ0	       0x08
#define IRQ1         0x09
#define IRQ2         0x0A
#define IRQ3         0x0B
#define IRQ4         0x0C
#define IRQ5         0x0D
#define IRQ6         0x0E
#define IRQ7         0x0F
#define IRQ8         0x70
#define IRQ9         0x71
#define IRQ10        0x72
#define IRQ11        0x73
#define IRQ12        0x74
#define IRQ13        0x75
#define IRQ14        0x76
#define IRQ15        0x77

#define IRQ_TIMER    IRQ0
#define IRQ_KEYBOARD IRQ1
#define IRQ_CASCADE  IRQ2
#define IRQ_SERIAL2  IRQ3
#define IRQ_SERIAL1  IRQ4
#define IRQ_PPORT2   IRQ5 // At systems: parallel port 2, PS/2 systems: reserved.
#define IRQ_DISKETTE IRQ6
#define IRQ_PPORT1   IRQ7
#define IRQ_CMOSCLK  IRQ8 // CMOS real time clock
#define IRQ_CGAVR    IRQ9 // CGA vertical retrace.
#define IRQ_AUX      IRQ12 // At systems: reserved, PS/2 systems: auxiliary device.
#define IRQ_FPU      IRQ13
#define IRQ_HDDCTRL  IRQ14 // Hard disk controller.

class Irq {
public:
  static void enable();
  static void disable();
  static bool isEnabled();
};

#endif // KERNEL_IRQ_H
