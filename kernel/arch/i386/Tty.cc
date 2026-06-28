#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

#ifdef DEBUG_THROUGH_SERIAL_COM1
#include <kernel/Serial.h>
#endif

namespace {

// Reserve row 0 for status indicator.
constexpr int SCROLLBACK_VIEW_HEIGHT = Tty::ROWS - 1;

static constinit uint8_t termRow = 0, termCol = 0, termColor = Tty::DEFAULT_COLOR;
static constinit bool scrolling = true;

// Scrollback ring buffer.
static constinit array<array<char, VGA_WIDTH + 1>, Tty::SCROLLBACK_LINES> scrollbackBuf_{};
static constinit int scrollbackHead_ = 0;
static constinit int scrollbackCount_ = 0;

static constinit bool scrollbackActive_ = false;
static constinit int scrollbackOffset_ = 0;

// Saved VGA RAM for restoring after scrollback exit.
static constinit array<array<uint16_t, VGA_WIDTH>, Tty::ROWS> savedScreen_{};

void clearRow(uint8_t row)
{
  termColor = Tty::DEFAULT_COLOR;
  fill_n(&VGA_RAM[row * VGA_WIDTH], VGA_WIDTH, vgaEntry(' ', termColor));
}

void swapRows(uint8_t row1, uint8_t row2)
{
  swap_ranges(&VGA_RAM[row1 * VGA_WIDTH], &VGA_RAM[row1 * VGA_WIDTH + VGA_WIDTH],
              &VGA_RAM[row2 * VGA_WIDTH]);
}

void advRow()
{
  // When reaching the bottom then scroll one line up instead of starting at the beginning and
  // overwriting things.
  if (++termRow == Tty::ROWS) {
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
      for (uint8_t r = 0; r < Tty::ROWS - 1; r++) {
        swapRows(r, r + 1);
      }

      termRow = Tty::ROWS - 1;
      termCol = 0;
    }
    else {
      termRow = 0;
    }
  }
}

void advCol()
{
  if (++termCol == VGA_WIDTH) {
    termCol = 0;
    advRow();
  }
}

void decCol()
{
  if (termCol > 0) {
    termCol--;
  }
  else if (termRow > 0) {
    termRow--;
    termCol = VGA_WIDTH - 1;
  }
}

// There are two VGA CRTC cursor-location registers: 0x3D4 for the index and 0x3D5 for data. The
// cursor location is set via the high register (bits 15-8) via 0x0E and the low register (bits 7-0)
// via 0x0F. The position is `row * VGA_WIDTH + col` (0..3999 for 80x25).
void cursorUpdate()
{
  const uint16_t pos = static_cast<uint16_t>(termRow * VGA_WIDTH + termCol);

  // High bits.
  Io::outb(0x3D4, 0x0E);
  Io::outb(0x3D5, static_cast<uint8_t>(pos >> 8));

  // Low bits.
  Io::outb(0x3D4, 0x0F);
  Io::outb(0x3D5, static_cast<uint8_t>(pos));
}

int rawPuts(const char *str)
{
  size_t i = 0;
  while (str[i]) {
#ifdef DEBUG_THROUGH_SERIAL_COM1
    if (str[i] != '\b') {
      Serial::write(str[i]);
    }
#endif
    if (str[i] == '\n') {
      advRow();
      termCol = 0;
    }
    else if (str[i] == '\r') {
      termCol = 0;
    }
    else if (str[i] == '\b') {
      decCol();
    }
    else {
      VGA_RAM[termRow * VGA_WIDTH + termCol] = vgaEntry(str[i], termColor);
      advCol();
    }
    ++i;
  }
  cursorUpdate();
  return static_cast<int>(i);
}

int rawPuts(const string &str)
{
  size_t i = 0;
  for (; i < str.size(); ++i) {
#ifdef DEBUG_THROUGH_SERIAL_COM1
    if (str[i] != '\b') {
      Serial::write(str[i]);
    }
#endif
    if (str[i] == '\n') {
      advRow();
      termCol = 0;
    }
    else if (str[i] == '\r') {
      termCol = 0;
    }
    else if (str[i] == '\b') {
      decCol();
    }
    else {
      VGA_RAM[termRow * VGA_WIDTH + termCol] = vgaEntry(str[i], termColor);
      advCol();
    }
  }
  cursorUpdate();
  return static_cast<int>(i);
}

} // namespace

Semaphore Tty::lock_(1);

void Tty::lock()
{
  lock_.wait();
}

void Tty::unlock()
{
  lock_.signal();
}

#ifndef __IS_DOORS_KERNEL
void Tty::resetLock()
{
  lock_ = Semaphore(1);
}
#endif

// The two VGA CRTC cursor-shape registers, start (0x0A) and end (0x0B), define scanlines of the
// cursor. With a 16-scanline font (numbered 0-15, top to bottom), scanlines 14-15 produce a
// blinking two-scanline underline.
void Tty::cursorEnable()
{
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
void Tty::cursorDisable()
{
  Io::outb(0x3D4, 0x0A);
  Io::outb(0x3D5, 0x20);
}

void Tty::cursorSetPos(uint8_t row, uint8_t col)
{
  lock();
  termRow = row;
  termCol = col;
  cursorUpdate();
  unlock();
}

void Tty::setColor(uint8_t color)
{
  lock();
  termColor = color;
  unlock();
}

void Tty::setScrolling(bool enabled)
{
  lock();
  scrolling = enabled;
  unlock();
}

void Tty::cls()
{
  lock();

  termRow = termCol = 0;
  termColor = Tty::DEFAULT_COLOR;
  fill_n(VGA_RAM, VGA_WIDTH * Tty::ROWS, vgaEntry(' ', termColor));
  cursorUpdate();

  unlock();
}

void Tty::putc(char ch)
{
  lock();

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
  else if (ch == '\r') {
    termCol = 0;
  }
  else if (ch == '\b') {
    decCol();
  }
  else {
    VGA_RAM[termRow * VGA_WIDTH + termCol] = vgaEntry(ch, termColor);
    advCol();
  }

  cursorUpdate();

  unlock();
}

void Tty::putc(char ch, uint8_t row, uint8_t col)
{
  lock();
  VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, termColor);
  unlock();
}

// NOTE: Both `const char*` and `string` overloads have their own loops rather than delegating one
// to the other, because `string(const char*)` calls `Heap::alloc()` when the string exceeds
// SSO. The panic/UBSan paths must never risk heap allocation!

int Tty::puts(const char *str)
{
  lock();
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

int Tty::puts(const char *str, uint8_t row, uint8_t col)
{
  lock();
  termRow = row;
  termCol = col;
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

int Tty::puts(const string &str)
{
  lock();
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

int Tty::puts(const string &str, uint8_t row, uint8_t col)
{
  lock();
  termRow = row;
  termCol = col;
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

void Tty::putLine(const char *str, uint8_t row)
{
  lock();

  // NOTE: Must use its own loop (not construct a string from str) because `string(const char*)` may
  // heap-allocate when SSO is exceeded.
  termRow = row;

  size_t i = 0;
  for (; str[i] && i < VGA_WIDTH; ++i) {
    VGA_RAM[row * VGA_WIDTH + i] = vgaEntry(str[i], termColor);
  }
  termCol = static_cast<uint8_t>(i);

  // Fill the rest of the line.
  fill_n(&VGA_RAM[row * VGA_WIDTH + i], VGA_WIDTH - i, vgaEntry(' ', termColor));

  cursorUpdate();

  unlock();
}

void Tty::putLine(const string &str, uint8_t row)
{
  lock();

  termRow = row;

  size_t i = 0;
  for (; i < str.size() && i < VGA_WIDTH; ++i) {
    VGA_RAM[row * VGA_WIDTH + i] = vgaEntry(str[i], termColor);
  }
  termCol = static_cast<uint8_t>(i);

  // Fill the rest of the line.
  fill_n(&VGA_RAM[row * VGA_WIDTH + i], VGA_WIDTH - i, vgaEntry(' ', termColor));

  cursorUpdate();

  unlock();
}

uint8_t Tty::getRow()
{
  lock();
  const uint8_t r = termRow;
  unlock();
  return r;
}

uint8_t Tty::getCol()
{
  lock();
  const uint8_t c = termCol;
  unlock();
  return c;
}

int Tty::scrollbackSize()
{
  return scrollbackCount_;
}

const char *Tty::scrollbackLine(int n)
{
  if (n < 0 || n >= scrollbackCount_) return nullptr;
  int idx = (scrollbackHead_ - 1 - n + SCROLLBACK_LINES) % SCROLLBACK_LINES;
  return scrollbackBuf_[idx].data();
}

void Tty::scrollbackShow(int offset)
{
  lock();

  if (offset < 1) {
    unlock();
    return;
  }
  if (offset > scrollbackCount_) {
    offset = scrollbackCount_;
  }

  // Save the current VGA RAM when first entering scrollback mode.
  if (!scrollbackActive_) {
    memcpy(savedScreen_.data(), VGA_RAM, sizeof(savedScreen_));
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

  // Build status indicator on row 0.
  char buf[64];
  snprintf(buf, sizeof(buf), "-- SCROLLBACK (offset %u of %u) --", static_cast<unsigned>(offset),
           static_cast<unsigned>(scrollbackCount_));

  const size_t slen = strlen(buf);
  for (size_t col = 0; col < VGA_WIDTH; col++) {
    const char ch = col < slen ? buf[col] : ' ';
    VGA_RAM[0 * VGA_WIDTH + col] = vgaEntry(ch, Tty::DEFAULT_COLOR);
  }

  // Display scrollback content on rows 1 to Tty::ROWS-1.
  for (size_t row = 1; row < Tty::ROWS; row++) {
    int lineFromEnd = top - static_cast<int>(row - 1);

    if (lineFromEnd > scrollbackCount_ || lineFromEnd < bottom) {
      fill_n(&VGA_RAM[row * VGA_WIDTH], VGA_WIDTH, vgaEntry(' ', Tty::DEFAULT_COLOR));
    }
    else {
      int actualIdx = (scrollbackHead_ - lineFromEnd + SCROLLBACK_LINES) % SCROLLBACK_LINES;
      const char *line = scrollbackBuf_[actualIdx].data();
      for (size_t col = 0; col < VGA_WIDTH; col++) {
        char ch = line[col] != '\0' ? line[col] : ' ';
        VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, Tty::DEFAULT_COLOR);
      }
    }
  }

  scrollbackActive_ = true;
  scrollbackOffset_ = offset;

  unlock();
}

void Tty::scrollbackExit()
{
  lock();

  if (!scrollbackActive_) {
    unlock();
    return;
  }
  scrollbackActive_ = false;
  scrollbackOffset_ = 0;
  memcpy(VGA_RAM, savedScreen_.data(), sizeof(savedScreen_));
  cursorEnable();
  Kbd::clearNavigation();

  unlock();
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
