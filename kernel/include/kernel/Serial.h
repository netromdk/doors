#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

class Serial {
public:
  static void init();
  static void write(char c);
};

#endif // KERNEL_SERIAL_H
