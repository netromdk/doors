#ifndef PROGRAMS_API_INPUT_H
#define PROGRAMS_API_INPUT_H

#include <cstdint>

class Input {
public:
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
    Char,
  };

  struct KeyEvent {
    Key key;
    char ch{0}; // Set when `key == Key::Char`.
  };

  static KeyEvent poll();
};

#endif // PROGRAMS_API_INPUT_H
