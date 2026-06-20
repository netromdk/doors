#ifndef KERNEL_KBD_H
#define KERNEL_KBD_H

#include <stddef.h>
#include <stdint.h>

struct HistoryCtx {
  char (*buf)[256];
  int size;
  int count;
  int head;
  int *pos;
};

class Kbd {
public:
  static constexpr size_t BUF_SIZE = 256;

  static constexpr char KEY_CTRL_C = 0x03; // Cancel current input.
  static constexpr char KEY_CTRL_N = 0x0E; // History down (alt to Down arrow).
  static constexpr char KEY_CTRL_P = 0x10; // History up (alt to Up arrow).
  static constexpr char KEY_CTRL_U = 0x15; // Erase entire input line.

  static void init();
  static void isrHandler();
  static void pushChar(char ch);
  static bool charAvail();
  static char getChar();
  static void readLine(char *buf, int max, HistoryCtx *history = nullptr);

  static void clearNavigation();

  // Test helpers for modifier simulation
  static void processScancode(uint8_t scancode, bool extended);
  static bool isShiftPressed();
  static bool isCtrlPressed();
  static bool isAltPressed();
  static bool isCapsLock();

  // Navigation event counters are set by ISR and consumed by `readLine()`.
  // Counters preserve rapid key presses (not flags).
  static volatile int pendingUp_;
  static volatile int pendingDown_;
  static volatile int pendingPageUp_;
  static volatile int pendingPageDown_;
  static volatile int pendingHome_;
  static volatile int pendingEnd_;

private:
  // Ring buffer
  static volatile char buffer_[BUF_SIZE];
  static volatile size_t head_;
  static volatile size_t tail_;

  // Modifier state
  static volatile bool shiftPressed_;
  static volatile bool ctrlPressed_;
  static volatile bool altPressed_;
  static volatile bool capsLock_;
};

#endif
