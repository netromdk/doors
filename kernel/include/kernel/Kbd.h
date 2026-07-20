#ifndef KERNEL_KBD_H
#define KERNEL_KBD_H

#include <cstddef>
#include <cstdint>
#include <string>

#include <kernel/Semaphore.h>

struct HistoryCtx {
  string *buf;
  int size;
  int count;
  int head;
  int *pos;
};

class Kbd {
public:
  static constexpr size_t BUF_SIZE = 256;

  static constexpr char KEY_CTRL_A = 0x01; // Cursor to start of line.
  static constexpr char KEY_CTRL_B = 0x02; // Cursor backward one char.
  static constexpr char KEY_CTRL_C = 0x03; // Cancel current input.
  static constexpr char KEY_CTRL_D = 0x04; // Delete character under cursor.
  static constexpr char KEY_CTRL_E = 0x05; // Cursor to end of line.
  static constexpr char KEY_CTRL_F = 0x06; // Cursor forward one char.
  static constexpr char KEY_CTRL_K = 0x0B; // Kill to end of line.
  static constexpr char KEY_CTRL_N = 0x0E; // History down (alt to Down arrow).
  static constexpr char KEY_CTRL_P = 0x10; // History up (alt to Up arrow).
  static constexpr char KEY_CTRL_U = 0x15; // Erase entire input line.

  static void init();
  static void isrHandler();
  static void pushChar(char ch);
  static bool charAvail();
  static char getChar();
  static void waitForChar();
  static void readLine(string &line, HistoryCtx *history = nullptr);

  static void clearNavigation();

  // Non-blocking input API.
  enum class Key : uint8_t {
    Unknown = 0,
    Up,
    Down,
    Left,
    Right,
    PageUp,
    PageDown,
    Home,
    End,
    Char
  };

  struct KeyEvent {
    Key key{};
    char ch{0}; // Set when `key == Key::Char`.
  };

  static KeyEvent tryReadKey();

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
  static volatile int pendingLeft_;
  static volatile int pendingRight_;
  static volatile int pendingPageUp_;
  static volatile int pendingPageDown_;
  static volatile int pendingHome_;
  static volatile int pendingEnd_;

private:
  static Semaphore available_;

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
