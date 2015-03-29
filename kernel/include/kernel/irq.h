#ifndef KERNEL_IRQ_H
#define KERNEL_IRQ_H

class Irq {
public:
  static void enable();
  static void disable();
  static bool isEnabled();
};

#endif // KERNEL_IRQ_H
