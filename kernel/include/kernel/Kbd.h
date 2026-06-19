#ifndef KERNEL_KBD_H
#define KERNEL_KBD_H

#include <stddef.h>
#include <stdint.h>

class Kbd {
public:
  static constexpr size_t BUF_SIZE = 256;

  static void init();
  static void isrHandler();
  static void pushChar(char ch);
  static bool charAvail();
  static char getChar();
  static void readLine(char *buf, int max);

  // Test helpers for modifier simulation
  static void processScancode(uint8_t scancode, bool extended);
  static bool isShiftPressed();
  static bool isCtrlPressed();
  static bool isAltPressed();
  static bool isCapsLock();

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
