#ifndef SYS_KBD_H
#define SYS_KBD_H

#include <cstdint>

// Keyboard protocol shared between the kernel and userland.
enum class KbdKey : uint8_t {
  Unknown = 0,
  Up,
  Down,
  Left,
  Right,
  PageUp,
  PageDown,
  Home,
  End,
  Char,
};

#endif
