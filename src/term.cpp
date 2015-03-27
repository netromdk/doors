#include "term.h"

size_t termRow, termCol;
uint8_t termColor;
uint16_t *termBuf;

void initTerm() {
  // Color text mode address.
  termBuf = (uint16_t*) 0xB8000;

  // Fill every entry with a blank space.
  termRow = termCol = 0;
  termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      termBuf[y * VGA_WIDTH + x] = vgaEntry(' ', termColor);
    }
  }
}
