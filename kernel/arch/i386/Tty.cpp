#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm.h>

#include <kernel/Kbd.h>
#include <kernel/Vga.h>
#include <kernel/Tty.h>
#include <kernel/Io.h>

#ifdef DEBUG_THROUGH_SERIAL_COM1
#include <kernel/Serial.h>
#endif

namespace {
  constexpr uint8_t DEFAULT_COLOR = vgaColor(COLOR_LIGHT_GREEN, COLOR_BLACK);

  // Reserve row 0 for status indicator.
  constexpr int SCROLLBACK_VIEW_HEIGHT = VGA_HEIGHT - 1;

  static constexpr char STATUS_PREFIX[] = "-- SCROLLBACK (offset ";
  static constexpr char STATUS_MIDDLE[] = " of ";
  static constexpr char STATUS_SUFFIX[] = ") --";

  static constinit uint8_t termRow = 0,
    termCol = 0,
    termColor = DEFAULT_COLOR;
  static constinit bool scrolling = true;

  // Scrollback ring buffer.
  static constinit char scrollbackBuf_[Tty::SCROLLBACK_LINES][VGA_WIDTH + 1]{};
  static constinit int scrollbackHead_ = 0;
  static constinit int scrollbackCount_ = 0;

  static constinit bool scrollbackActive_ = false;
  static constinit int scrollbackOffset_ = 0;

  // Saved VGA RAM for restoring after scrollback exit.
  static constinit uint16_t savedScreen_[VGA_HEIGHT][VGA_WIDTH]{};

  void clearRow(uint8_t row) {
    termColor = DEFAULT_COLOR;
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
    // When reaching the bottom then scroll one line up instead of starting at the beginning and
    // overwriting things.
    if (++termRow == VGA_HEIGHT) {
      if (scrolling) {
        // Save row 0, that is about to scroll off, into the scrollback buffer.
        for (size_t c = 0; c < VGA_WIDTH; c++) {
          scrollbackBuf_[scrollbackHead_][c] = static_cast<char>(VGA_RAM[c] & 0xFF);
        }
        scrollbackBuf_[scrollbackHead_][VGA_WIDTH] = '\0';
        scrollbackHead_ = (scrollbackHead_ + 1) % Tty::SCROLLBACK_LINES;
        if (scrollbackCount_ < Tty::SCROLLBACK_LINES) {
          scrollbackCount_++;
        }

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
  termColor = DEFAULT_COLOR;
  size_t total = VGA_WIDTH * VGA_HEIGHT;
  for (size_t i = 0; i < total; i++) {
    VGA_RAM[i] = vgaEntry(' ', termColor);
  }
  cursorUpdate();
}

void Tty::putc(char ch) {
  // Exit scrollback view on any character input.
  if (scrollbackActive_) {
    scrollbackExit();
  }

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

int Tty::scrollbackSize()
{
  return scrollbackCount_;
}

const char *Tty::scrollbackLine(int n)
{
  if (n < 0 || n >= scrollbackCount_) return nullptr;
  int idx = (scrollbackHead_ - 1 - n + SCROLLBACK_LINES) % SCROLLBACK_LINES;
  return scrollbackBuf_[idx];
}

void Tty::scrollbackShow(int offset)
{
  if (offset < 1) return;
  if (offset > scrollbackCount_) {
    offset = scrollbackCount_;
  }

  // Save the current VGA RAM when first entering scrollback mode.
  if (!scrollbackActive_) {
    memcpy(savedScreen_, VGA_RAM, sizeof(savedScreen_));
  }

  cursorDisable();

  // Compute display range:
  //   `offset` is the newest line to show, which is at the bottom of the content area.
  //   `top` is the oldest line to show, which is at the top of the content area.
  //   Row 0 is reserved for the status indicator.
  int top = offset + SCROLLBACK_VIEW_HEIGHT - 1;
  if (top > scrollbackCount_) {
    top = scrollbackCount_;
  }

  int bottom = top - SCROLLBACK_VIEW_HEIGHT + 1;
  if (bottom < 1) {
    bottom = 1;
  }

  // Update offset to reflect the actual bottom line of the content area.
  offset = bottom;

  // Write status indicator on row 0.
  char buf[VGA_WIDTH + 1];
  size_t pos = 0;
  for (size_t i = 0; STATUS_PREFIX[i] != '\0' && pos < VGA_WIDTH; i++) {
    buf[pos++] = STATUS_PREFIX[i];
  }

  // Append offset number.
  if (offset >= 10000) buf[pos++] = '0' + (offset / 10000) % 10;
  if (offset >= 1000) buf[pos++] = '0' + (offset / 1000) % 10;
  if (offset >= 100) buf[pos++] = '0' + (offset / 100) % 10;
  if (offset >= 10) buf[pos++] = '0' + (offset / 10) % 10;

  // Write middle indicator text.
  buf[pos++] = '0' + offset % 10;
  for (size_t i = 0; STATUS_MIDDLE[i] != '\0' && pos < VGA_WIDTH; i++) {
    buf[pos++] = STATUS_MIDDLE[i];
  }

  // Append scrollbackCount_ number.
  int total = scrollbackCount_;
  if (total >= 10000) buf[pos++] = '0' + (total / 10000) % 10;
  if (total >= 1000) buf[pos++] = '0' + (total / 1000) % 10;
  if (total >= 100) buf[pos++] = '0' + (total / 100) % 10;
  if (total >= 10) buf[pos++] = '0' + (total / 10) % 10;

  // Write suffix indicator text.
  buf[pos++] = '0' + total % 10;
  for (size_t i = 0; STATUS_SUFFIX[i] != '\0' && pos < VGA_WIDTH; i++) {
    buf[pos++] = STATUS_SUFFIX[i];
  }

  // Display the indicator on VGA.
  for (size_t col = 0; col < VGA_WIDTH; col++) {
    char ch = col < pos ? buf[col] : ' ';
    VGA_RAM[0 * VGA_WIDTH + col] = vgaEntry(ch, DEFAULT_COLOR);
  }

  // Display scrollback content on rows 1 to VGA_HEIGHT-1.
  for (size_t row = 1; row < VGA_HEIGHT; row++) {
    int lineFromEnd = top - static_cast<int>(row - 1);

    if (lineFromEnd > scrollbackCount_ || lineFromEnd < bottom) {
      for (size_t col = 0; col < VGA_WIDTH; col++) {
        VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(' ', DEFAULT_COLOR);
      }
    }
    else {
      int actualIdx = (scrollbackHead_ - lineFromEnd + SCROLLBACK_LINES) % SCROLLBACK_LINES;
      const char *line = scrollbackBuf_[actualIdx];
      for (size_t col = 0; col < VGA_WIDTH; col++) {
        char ch = line[col] != '\0' ? line[col] : ' ';
        VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, DEFAULT_COLOR);
      }
    }
  }

  scrollbackActive_ = true;
  scrollbackOffset_ = offset;
}

void Tty::scrollbackExit()
{
  if (!scrollbackActive_) return;
  scrollbackActive_ = false;
  scrollbackOffset_ = 0;
  memcpy(VGA_RAM, savedScreen_, sizeof(savedScreen_));
  cursorEnable();
  Kbd::clearNavigation();
}

bool Tty::scrollbackActive()
{
  return scrollbackActive_;
}

void Tty::scrollbackPageUp()
{
  int offset = scrollbackActive_ ? scrollbackOffset_ : 0;
  offset += SCROLLBACK_VIEW_HEIGHT;
  if (offset > scrollbackCount_) {
    offset = scrollbackCount_;
  }
  scrollbackShow(offset);
}

void Tty::scrollbackHome()
{
  if (scrollbackCount_ == 0) return;
  scrollbackShow(scrollbackCount_);
}

int Tty::scrollbackOffset()
{
  return scrollbackActive_ ? scrollbackOffset_ : 0;
}

void Tty::scrollbackLineUp()
{
  if (!scrollbackActive_) return;
  int offset = scrollbackOffset_ + 1;
  if (offset > scrollbackCount_) {
    return;
  }
  scrollbackShow(offset);
}

void Tty::scrollbackLineDown()
{
  if (!scrollbackActive_) return;
  int offset = scrollbackOffset_ - 1;
  if (offset <= 0) {
    scrollbackExit();
    return;
  }
  scrollbackShow(offset);
}

void Tty::scrollbackPageDown()
{
  if (!scrollbackActive_) return;
  int offset = scrollbackOffset_ - SCROLLBACK_VIEW_HEIGHT;
  if (offset <= 0) {
    scrollbackExit();
    return;
  }
  scrollbackShow(offset);
}
