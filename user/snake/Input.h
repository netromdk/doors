#ifndef USER_SNAKE_SNAKEINPUT_H
#define USER_SNAKE_SNAKEINPUT_H

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
    char ch{0};
  };

  static KeyEvent poll();
};

#endif // USER_SNAKE_SNAKEINPUT_H
