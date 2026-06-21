#ifndef KERNEL_KEYMAP_H
#define KERNEL_KEYMAP_H

#include <cstdint>

enum class Key : uint32_t {
  // ASCII control codes
  Backspace = 0x08,
  Tab = 0x09,
  Enter = 0x0A,
  Escape = 0x1B,

  // Printable ASCII
  Space = 0x20,

  _0 = 0x30,
  _1,
  _2,
  _3,
  _4,
  _5,
  _6,
  _7,
  _8,
  _9,

  A = 0x41,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,

  // Symbol keys (physical key labels, toText maps to actual chars)
  Minus,     // 0x5B
  Equals,    // 0x5C
  LBracket,  // 0x5D
  RBracket,  // 0x5E
  Semicolon, // 0x5F
  Quote,     // 0x60
  Backtick,  // 0x61
  Backslash, // 0x62
  Comma,     // 0x63
  Period,    // 0x64
  Slash,     // 0x65

  // Modifier keys
  LShift, // 0x66
  RShift,
  LCtrl,
  RCtrl,
  LAlt,
  RAlt,
  CapsLock,
  NumLock,
  ScrollLock,

  // Navigation
  Up,
  Down,
  Left,
  Right,
  Home,
  End,
  PageUp,
  PageDown,
  Insert,
  Delete,

  // Function keys
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  // Keypad
  Keypad0,
  Keypad1,
  Keypad2,
  Keypad3,
  Keypad4,
  Keypad5,
  Keypad6,
  Keypad7,
  Keypad8,
  Keypad9,
  KeypadPeriod,
  KeypadAsterisk,
  KeypadMinus,
  KeypadPlus,

  // Extended keys
  LGui,
  RGui,
  Apps,

  Unknown = 0x1FFFFFFF,
};

class KeyMap {
public:
  static Key scancodeToKey(uint8_t scancode, bool extended);
  static char toText(Key key, bool shift, bool ctrl, bool caps);
};

#endif // KERNEL_KEYMAP_H
