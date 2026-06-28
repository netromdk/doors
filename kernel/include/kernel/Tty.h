#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <cstdint>
#include <kernel/Semaphore.h>
#include <kernel/Vga.h>

class string;

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

#ifndef __IS_DOORS_KERNEL
  // Reset the internal semaphore to a clean binary state (used by tests).
  static void resetLock();
#endif

  static void cls();

  static void putc(char ch);
  static void putc(char ch, uint8_t row, uint8_t col);

  static int puts(const char *str);
  static int puts(const char *str, uint8_t row, uint8_t col);
  static int puts(const string &str);
  static int puts(const string &str, uint8_t row, uint8_t col);

  static void putLine(const char *str, uint8_t row);
  static void putLine(const string &str, uint8_t row);

  static void cursorEnable();
  static void cursorDisable();
  static void cursorSetPos(uint8_t row, uint8_t col);

  static uint8_t getRow();
  static uint8_t getCol();

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
  static Semaphore lock_;
};

#endif // KERNEL_TTY_H
