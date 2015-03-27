#include "term.h"
#include "string.h"

size_t termRow, termCol;
uint8_t termColor;
uint16_t *termBuf;

void initTerm() {
  // Color text mode address.
  termBuf = (uint16_t*) 0xB8000;

  // Fill every entry with a blank space.
  termRow = termCol = 0;
  termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
  size_t total = VGA_WIDTH * VGA_HEIGHT;
  for (size_t i = 0; i < total; i++) {
    putc(' ');
  }
  // At this point (row,col) has been reset to (0,0).
}

void setTermColor(uint8_t color) {
  termColor = color;
}

namespace {
  void advRow() {
    if (++termRow == VGA_HEIGHT) {
      termRow = 0;
    }
  }

  void advCol() {
    if (++termCol == VGA_WIDTH) {
      termCol = 0;
      advRow();
    }
  }
}

void putc(char ch) {
  if (ch == '\n') {
    advRow();
    termCol = 0;
    return;
  }

  termBuf[termRow * VGA_WIDTH + termCol] = vgaEntry(ch, termColor);
  advCol();
}

void putstr(const char *str) {
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    putc(str[i]);
  }
}
