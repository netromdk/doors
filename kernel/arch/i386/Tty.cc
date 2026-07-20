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

Semaphore Tty::lock_(1);
constinit uint8_t Tty::termRow_ = 0;
constinit uint8_t Tty::termCol_ = 0;
constinit uint8_t Tty::termColor_ = Tty::DEFAULT_COLOR;
constinit bool Tty::scrolling_ = true;
constinit array<array<char, VGA_WIDTH + 1>, Tty::SCROLLBACK_LINES> Tty::scrollbackBuf_{};
constinit int Tty::scrollbackHead_ = 0;
constinit int Tty::scrollbackCount_ = 0;
constinit bool Tty::scrollbackActive_ = false;
constinit int Tty::scrollbackOffset_ = 0;
constinit array<array<uint16_t, VGA_WIDTH>, Tty::ROWS> Tty::savedScreen_{};

void Tty::clearRow(uint8_t row)
{
  termColor_ = Tty::DEFAULT_COLOR;
  fill_n(&VGA_RAM[row * VGA_WIDTH], VGA_WIDTH, vgaEntry(' ', termColor_));
}

void Tty::swapRows(uint8_t row1, uint8_t row2)
{
  swap_ranges(&VGA_RAM[row1 * VGA_WIDTH], &VGA_RAM[row1 * VGA_WIDTH + VGA_WIDTH],
              &VGA_RAM[row2 * VGA_WIDTH]);
}

void Tty::advRow()
{
  // When reaching the bottom then scroll one line up instead of starting at the beginning and
  // overwriting things.
  if (++termRow_ == ROWS) {
    if (scrolling_) {
      for (size_t c = 0; c < VGA_WIDTH; c++) {
        scrollbackBuf_[scrollbackHead_][c] = static_cast<char>(VGA_RAM[c] & 0xFF);
      }
      scrollbackBuf_[scrollbackHead_][VGA_WIDTH] = '\0';
      scrollbackHead_ = (scrollbackHead_ + 1) % SCROLLBACK_LINES;
      if (scrollbackCount_ < SCROLLBACK_LINES) {
        scrollbackCount_++;
      }

      clearRow(0);
      for (uint8_t r = 0; r < ROWS - 1; r++) {
        swapRows(r, r + 1);
      }

      termRow_ = ROWS - 1;
      termCol_ = 0;
    }
    else {
      termRow_ = 0;
    }
  }
}

void Tty::advCol()
{
  if (++termCol_ == VGA_WIDTH) {
    termCol_ = 0;
    advRow();
  }
}

void Tty::decCol()
{
  if (termCol_ > 0) {
    termCol_--;
  }
  else if (termRow_ > 0) {
    termRow_--;
    termCol_ = VGA_WIDTH - 1;
  }
}

void Tty::cursorUpdate()
{
  const uint16_t pos = static_cast<uint16_t>(termRow_ * VGA_WIDTH + termCol_);

  Io::outb(0x3D4, 0x0E);
  Io::outb(0x3D5, static_cast<uint8_t>(pos >> 8));

  Io::outb(0x3D4, 0x0F);
  Io::outb(0x3D5, static_cast<uint8_t>(pos));
}

int Tty::rawPuts(string_view sv)
{
  for (size_t i = 0; i < sv.size(); ++i) {
#ifdef DEBUG_THROUGH_SERIAL_COM1
    if (sv[i] != '\b') {
      Serial::write(sv[i]);
    }
#endif
    if (sv[i] == '\n') {
      advRow();
      termCol_ = 0;
    }
    else if (sv[i] == '\r') {
      termCol_ = 0;
    }
    else if (sv[i] == '\b') {
      decCol();
    }
    else {
      VGA_RAM[termRow_ * VGA_WIDTH + termCol_] = vgaEntry(sv[i], termColor_);
      advCol();
    }
  }
  cursorUpdate();
  return static_cast<int>(sv.size());
}

void Tty::lock()
{
  lock_.wait();
}

void Tty::unlock()
{
  lock_.signal();
}

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
  termRow_ = row;
  termCol_ = col;
  cursorUpdate();
  unlock();
}

void Tty::setColor(uint8_t color)
{
  lock();
  termColor_ = color;
  unlock();
}

void Tty::setScrolling(bool enabled)
{
  lock();
  scrolling_ = enabled;
  unlock();
}

void Tty::cls()
{
  lock();

  termRow_ = termCol_ = 0;
  termColor_ = Tty::DEFAULT_COLOR;
  fill_n(VGA_RAM, VGA_WIDTH * ROWS, vgaEntry(' ', termColor_));
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
    termCol_ = 0;
  }
  else if (ch == '\r') {
    termCol_ = 0;
  }
  else if (ch == '\b') {
    decCol();
  }
  else {
    VGA_RAM[termRow_ * VGA_WIDTH + termCol_] = vgaEntry(ch, termColor_);
    advCol();
  }

  cursorUpdate();

  unlock();
}

void Tty::putc(char ch, uint8_t row, uint8_t col)
{
  lock();
  VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, termColor_);
  unlock();
}

int Tty::puts(string_view str)
{
  lock();
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

int Tty::puts(string_view str, uint8_t row, uint8_t col)
{
  lock();
  termRow_ = row;
  termCol_ = col;
  const int ret = rawPuts(str);
  unlock();
  return ret;
}

void Tty::putLine(string_view str, uint8_t row)
{
  lock();

  termRow_ = row;

  size_t i = 0;
  for (; i < str.size() && i < VGA_WIDTH; ++i) {
    VGA_RAM[row * VGA_WIDTH + i] = vgaEntry(str[i], termColor_);
  }
  termCol_ = static_cast<uint8_t>(i);

  // Fill the rest of the line.
  fill_n(&VGA_RAM[row * VGA_WIDTH + i], VGA_WIDTH - i, vgaEntry(' ', termColor_));

  cursorUpdate();

  unlock();
}

pair<uint8_t, uint8_t> Tty::getCursor()
{
  lock();
  const auto p = pair<uint8_t, uint8_t>{termRow_, termCol_};
  unlock();
  return p;
}

int Tty::scrollbackSize()
{
  return scrollbackCount_;
}

const char *Tty::scrollbackLine(int n)
{
  if (n < 0 || n >= scrollbackCount_) {
    return nullptr;
  }
  const int idx = (scrollbackHead_ - 1 - n + SCROLLBACK_LINES) % SCROLLBACK_LINES;
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
    memcpy(savedScreen_.data(), VGA_RAM, ROWS * VGA_WIDTH * sizeof(uint16_t));
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
    const int lineFromEnd = top - static_cast<int>(row - 1);

    if (lineFromEnd > scrollbackCount_ || lineFromEnd < bottom) {
      fill_n(&VGA_RAM[row * VGA_WIDTH], VGA_WIDTH, vgaEntry(' ', Tty::DEFAULT_COLOR));
    }
    else {
      const int actualIdx = (scrollbackHead_ - lineFromEnd + SCROLLBACK_LINES) % SCROLLBACK_LINES;
      const char *line = scrollbackBuf_[actualIdx].data();
      for (size_t col = 0; col < VGA_WIDTH; col++) {
        const char ch = line[col] != '\0' ? line[col] : ' ';
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
  memcpy(VGA_RAM, savedScreen_.data(), ROWS * VGA_WIDTH * sizeof(uint16_t));
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
  if (scrollbackCount_ == 0) {
    return;
  }
  scrollbackShow(scrollbackCount_);
}

int Tty::scrollbackOffset()
{
  return scrollbackActive_ ? scrollbackOffset_ : 0;
}

void Tty::scrollbackLineUp()
{
  if (!scrollbackActive_) {
    return;
  }
  const int offset = scrollbackOffset_ + 1;
  if (offset > scrollbackCount_) {
    return;
  }
  scrollbackShow(offset);
}

void Tty::scrollbackLineDown()
{
  if (!scrollbackActive_) {
    return;
  }
  const int offset = scrollbackOffset_ - 1;
  if (offset <= 0) {
    scrollbackExit();
    return;
  }
  scrollbackShow(offset);
}

void Tty::scrollbackPageDown()
{
  if (!scrollbackActive_) {
    return;
  }
  const int offset = scrollbackOffset_ - SCROLLBACK_VIEW_HEIGHT;
  if (offset <= 0) {
    scrollbackExit();
    return;
  }
  scrollbackShow(offset);
}
