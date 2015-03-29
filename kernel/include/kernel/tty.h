#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <stdint.h>

class Tty {
public:
  static void setColor(uint8_t color);
  static void setScrolling(bool enabled = true);

  static void cls();

  static void putc(char ch);
  static void putc(char ch, uint8_t row, uint8_t col);

  static int puts(const char *str);
  static int puts(const char *str, uint8_t row, uint8_t col);
};

#endif // KERNEL_TTY_H
