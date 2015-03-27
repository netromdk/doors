#ifndef TERM_H
#define TERM_H

#include <stddef.h>
#include <stdint.h>

// VGA standard terminal dimensions.
static const size_t VGA_WIDTH = 80,
  VGA_HEIGHT = 25;

// VGA standard hardware colors.
enum VgaColor {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

inline uint8_t vgaColor(VgaColor fg, VgaColor bg) {
  return fg | bg << 4;
}

inline uint16_t vgaEntry(char ch, uint8_t color) {
  return uint16_t(ch) | uint16_t(color) << 8;
}

void initTerm();

void setTermColor(uint8_t color);

void putc(char ch);
void putstr(const char *str);

#endif // TERM_H
