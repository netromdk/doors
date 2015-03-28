#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <stdint.h>

void cls();

void setTermColor(uint8_t color);
void setTermScrolling(bool enabled = true);

void putc(char ch);
void putc(char ch, uint8_t row, uint8_t col);

void puts(const char *str);
void puts(const char *str, uint8_t row, uint8_t col);

void printf(const char *format, ...);

#endif // KERNEL_TTY_H
