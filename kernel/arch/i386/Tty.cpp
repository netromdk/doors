#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm.h>

#include <kernel/Vga.h>
#include <kernel/Tty.h>

namespace {
  uint8_t termRow = 0,
    termCol = 0,
    termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
  bool scrolling = true;

  void clearRow(uint8_t row) {
    termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
    for (uint8_t col = 0; col < VGA_WIDTH; col++) {
      Tty::putc(' ', row, col);
    }
  }

  void swapRows(uint8_t row1, uint8_t row2) {
    uint16_t *line1 = &VGA_RAM[row1 * VGA_WIDTH],
      *line2 = &VGA_RAM[row2 * VGA_WIDTH];
    for (uint8_t i = 0; i < VGA_WIDTH; i++) {
      swap(line1[i], line2[i]);
    }
  }

  void advRow() {
    // When reaching the bottom then scroll one line up instead of
    // starting at the beginning and overwriting things.
    if (++termRow == VGA_HEIGHT) {
      if (scrolling) {
        clearRow(0);
        for (uint8_t r = 0; r < VGA_HEIGHT - 1; r++) {
          swapRows(r, r + 1);
        }

        termRow = VGA_HEIGHT - 1;
        termCol = 0;
      }
      else {
        termRow = 0;
      }
    }
  }

  void advCol() {
    if (++termCol == VGA_WIDTH) {
      termCol = 0;
      advRow();
    }
  }
}

void Tty::setColor(uint8_t color) {
  termColor = color;
}

void Tty::setScrolling(bool enabled) {
  scrolling = enabled;
}

void Tty::cls() {
  termRow = termCol = 0;
  termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
  size_t total = VGA_WIDTH * VGA_HEIGHT;
  for (size_t i = 0; i < total; i++) {
    putc(' ');
  }
  termRow = termCol = 0;
}

void Tty::putc(char ch) {
  if (ch == '\n') {
    advRow();
    termCol = 0;
    return;
  }

  putc(ch, termRow, termCol);
  advCol();
}

void Tty::putc(char ch, uint8_t row, uint8_t col) {
  VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, termColor);
}

int Tty::puts(const char *str) {
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    putc(str[i]);
  }
  return len;
}

int Tty::puts(const char *str, uint8_t row, uint8_t col) {
  termRow = row;
  termCol = col;
  return puts(str);
}
