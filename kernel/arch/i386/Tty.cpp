#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm.h>

#include <kernel/Vga.h>
#include <kernel/Tty.h>
#include <kernel/Io.h>

#ifdef DEBUG_THROUGH_SERIAL_COM1
#include <kernel/Serial.h>
#endif

namespace {
  static constinit uint8_t termRow = 0,
    termCol = 0,
    termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
  static constinit bool scrolling = true;

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

  void decCol() {
    if (termCol > 0) {
      termCol--;
    }
    else if (termRow > 0) {
      termRow--;
      termCol = VGA_WIDTH - 1;
    }
  }

  // There are two VGA CRTC cursor-location registers: 0x3D4 for the index and 0x3D5 for data. The
  // cursor location is set via the high register (bits 15-8) via 0x0E and the low register (bits
  // 7-0) via 0x0F. The position is `row * VGA_WIDTH + col` (0..3999 for 80x25).
  void cursorUpdate() {
    const uint16_t pos = static_cast<uint16_t>(termRow * VGA_WIDTH + termCol);

    // High bits.
    Io::outb(0x3D4, 0x0E);
    Io::outb(0x3D5, static_cast<uint8_t>(pos >> 8));

    // Low bits.
    Io::outb(0x3D4, 0x0F);
    Io::outb(0x3D5, static_cast<uint8_t>(pos));
  }
}

// The two VGA CRTC cursor-shape registers, start (0x0A) and end (0x0B), define scanlines of the
// cursor. With a 16-scanline font (numbered 0-15, top to bottom), scanlines 14-15 produce a
// blinking two-scanline underline.
void Tty::cursorEnable() {
  // Cursor start at scanline 14.
  // 0x0E (0b00001110) meaning 5th bit is 0 (cursor is visible) and bits 4-0 (0b01110 = 14) is the
  // scanline.
  Io::outb(0x3D4, 0x0A);
  Io::outb(0x3D5, 0x0E);

  // Cursor end at scanline 15.
  // 0x0F (0b00001111) also has 5th bit = 0 (visible) and bits 4-0 (0b01111 = 15) is the
  // scanline.
  Io::outb(0x3D4, 0x0B);
  Io::outb(0x3D5, 0x0F);
  cursorUpdate();
}

// Writing 0x20 (0b100000) to the Cursor Start Register sets bit 5, which hides the cursor without
// affecting its position.
void Tty::cursorDisable() {
  Io::outb(0x3D4, 0x0A);
  Io::outb(0x3D5, 0x20);
}

void Tty::cursorSetPos(uint8_t row, uint8_t col) {
  termRow = row;
  termCol = col;
  cursorUpdate();
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
    VGA_RAM[i] = vgaEntry(' ', termColor);
  }
  cursorUpdate();
}

void Tty::putc(char ch) {
  // Write all characters through serial COM1 when enabled for debugging purposes.
#ifdef DEBUG_THROUGH_SERIAL_COM1
  Serial::write(ch);
#endif

  if (ch == '\n') {
    advRow();
    termCol = 0;
  }
  else if (ch == '\b') {
    decCol();
  }
  else {
    putc(ch, termRow, termCol);
    advCol();
  }

  cursorUpdate();
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

uint8_t Tty::getRow()
{
  return termRow;
}

uint8_t Tty::getCol()
{
  return termCol;
}
