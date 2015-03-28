#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <stdint.h>

void setTermColor(uint8_t color);
void setTermScrolling(bool enabled = true);

void termCls();

void termPutc(char ch);
void termPutc(char ch, uint8_t row, uint8_t col);

int termPuts(const char *str);
int termPuts(const char *str, uint8_t row, uint8_t col);

#endif // KERNEL_TTY_H
