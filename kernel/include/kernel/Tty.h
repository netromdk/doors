#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <kernel/Semaphore.h>
#include <kernel/Vga.h>
#include <string_view>
#include <utility>

class Tty {
public:
  static constexpr uint8_t DEFAULT_COLOR = vgaColor(COLOR_LIGHT_GREEN, COLOR_BLACK);

  // Scrollable rows (0-ROWS-1). The last row is reserved for taskbar!
  static constexpr int ROWS = VGA_HEIGHT - 1;

  static constexpr int SCROLLBACK_LINES = 1000;

  static void setColor(uint8_t color);
  static void setScrolling(bool enabled = true);

  static void lock();
  static void unlock();

  static void cls();

  static void putc(char ch);
  static void putc(char ch, uint8_t row, uint8_t col);

  static int puts(string_view str);
  static int puts(string_view str, uint8_t row, uint8_t col);

  static void putLine(string_view str, uint8_t row);

  static void cursorEnable();
  static void cursorDisable();
  static void cursorSetPos(uint8_t row, uint8_t col);

  static pair<uint8_t, uint8_t> getCursor();

  // Scrollback buffer
  static int scrollbackSize();
  static const char *scrollbackLine(int n);
  static void scrollbackShow(int offset);
  static void scrollbackExit();
  static bool scrollbackActive();
  static void scrollbackPageUp();
  static void scrollbackPageDown();
  static void scrollbackLineUp();
  static void scrollbackLineDown();
  static void scrollbackHome();
  static int scrollbackOffset();

private:
  static constexpr int SCROLLBACK_VIEW_HEIGHT = ROWS - 1;

  static Semaphore lock_;
  static uint8_t termRow_;
  static uint8_t termCol_;
  static uint8_t termColor_;
  static bool scrolling_;
  static array<array<char, VGA_WIDTH + 1>, SCROLLBACK_LINES> scrollbackBuf_;
  static int scrollbackHead_;
  static int scrollbackCount_;
  static bool scrollbackActive_;
  static int scrollbackOffset_;
  static array<array<uint16_t, VGA_WIDTH>, ROWS> savedScreen_;

  static void clearRow(uint8_t row);
  static void swapRows(uint8_t row1, uint8_t row2);
  static void advRow();
  static void advCol();
  static void decCol();
  static void cursorUpdate();
  static int rawPuts(string_view sv);

#ifndef __IS_DOORS_KERNEL
  friend struct TtyTestAccess;
#endif
};

#endif // KERNEL_TTY_H
