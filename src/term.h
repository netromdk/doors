#ifndef TERM_H
#define TERM_H

#include <stddef.h>
#include <stdint.h>

// VGA standard terminal dimensions.
static const size_t VGA_WIDTH = 80,
  VGA_HEIGHT = 25;

// VGA standard hardware colors.
enum VgaColor {
  COLOR_BLACK         = 0x0,
  COLOR_BLUE          = 0x1,
  COLOR_GREEN         = 0x2,
  COLOR_CYAN          = 0x3,
  COLOR_RED           = 0x4,
  COLOR_MAGENTA       = 0x5,
  COLOR_BROWN         = 0x6,
  COLOR_LIGHT_GREY    = 0x7,
  COLOR_DARK_GREY     = 0x8,
  COLOR_LIGHT_BLUE    = 0x9,
  COLOR_LIGHT_GREEN   = 0xA,
  COLOR_LIGHT_CYAN    = 0xB,
  COLOR_LIGHT_RED     = 0xC,
  COLOR_LIGHT_MAGENTA = 0xD,
  COLOR_LIGHT_BROWN   = 0xE,
  COLOR_WHITE         = 0xF
};

inline uint8_t vgaColor(VgaColor fg, VgaColor bg) {
  return fg | bg << 4;
}

inline uint16_t vgaEntry(char ch, uint8_t color) {
  return uint16_t(ch) | uint16_t(color) << 8;
}

void cls();

void setTermColor(uint8_t color);
void setTermScrolling(bool enabled = true);

void putc(char ch);
void putc(char ch, uint8_t row, uint8_t col);

void puts(const char *str);
void puts(const char *str, uint8_t row, uint8_t col);

void printf(const char *format, ...);

#endif // TERM_H
