#ifndef USER_SNAKE_SNAKEINPUT_H
#define USER_SNAKE_SNAKEINPUT_H

#include <cstdint>

#include <sys/kbd.h>

class Input {
public:
  using Key = KbdKey;

  struct KeyEvent {
    Key key{};
    char ch{0};
  };

  static KeyEvent poll();
};

#endif // USER_SNAKE_SNAKEINPUT_H
