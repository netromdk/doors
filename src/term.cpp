#include "term.h"
#include "stdlib.h"
#include "string.h"

namespace term {
  uint8_t termRow = 0,
    termCol = 0,
    termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);

  // Address of VRAM - color text mode.
  uint16_t *vram = (uint16_t*) 0xB8000;

  void cls() {
    termRow = termCol = 0;
    termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
    size_t total = VGA_WIDTH * VGA_HEIGHT;
    for (size_t i = 0; i < total; i++) {
      putc(' ');
    }
    termRow = termCol = 0;
  }

  void setColor(uint8_t color) {
    termColor = color;
  }

  namespace {
    void clearRow(uint8_t row) {
      termColor = vgaColor(COLOR_WHITE, COLOR_BLACK);
      for (uint8_t col = 0; col < VGA_WIDTH; col++) {
        putc(' ', row, col);
      }
    }

    void swapRows(uint8_t row1, uint8_t row2) {
      uint16_t *line1 = &vram[row1 * VGA_WIDTH],
        *line2 = &vram[row2 * VGA_WIDTH];
      for (uint8_t i = 0; i < VGA_WIDTH; i++) {
        swap(line1[i], line2[i]);
      }
    }

    void advRow() {
      // When reaching the bottom then scroll one line up instead of
      // starting at the beginning and overwriting things.
      if (++termRow == VGA_HEIGHT) {
        clearRow(0);
        for (uint8_t r = 0; r < VGA_HEIGHT - 1; r++) {
          swapRows(r, r + 1);
        }

        termRow = VGA_HEIGHT - 1;
        termCol = 0;
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

    putc(ch, termRow, termCol);
    advCol();
  }

  void putc(char ch, uint8_t row, uint8_t col) {
    vram[row * VGA_WIDTH + col] = vgaEntry(ch, termColor);
  }

  void puts(const char *str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
      putc(str[i]);
    }
  }

  void puts(const char *str, uint8_t row, uint8_t col) {
    termRow = row;
    termCol = col;
    puts(str);
  }

  void print(const char *format, ...) {
    char **args = (char**) &format;
    args++;

    char buf[64];
    char c;
    while ((c = *format++)) {
      if (c != '%') {
        putc(c);
        continue;
      }

      char* tmp; // holder of temp. strings
      c = *format++; // get format character

      switch (c) {
      case 's': // string
        tmp = *args++;
        if (!tmp) {
          tmp = (char*) "(NULL)";
        }
        puts(tmp);
        break;

      case 'c': // character
        putc((char) *((int32_t*) args++));
        break;

      case 'b': // binary
        itos(*((int32_t*) args++), buf, 2);
        puts(buf);
        break;

      case 'o': // octal
        itos(*((int32_t*) args++), buf, 8);
        puts(buf);
        break;

      case 'd': // decimal
        itos(*((int32_t*) args++), buf, 10);
        puts(buf);
        break;

      case 'x': // hexadecimal
        itos(*((int32_t*) args++), buf, 16);
        puts(buf);
        break;

      case 'u': // unsigned integer
        utos(*((uint32_t*) args++), buf);
        puts(buf);
        break;
      }
    }
  }
}
