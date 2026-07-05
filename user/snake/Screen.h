#ifndef USER_SNAKE_SNAKESCREEN_H
#define USER_SNAKE_SNAKESCREEN_H

#include <cstdint>

class Screen {
public:
  static void put(int row, int col, char ch, uint8_t color);
  static void save();
  static void restore();
  static void cls(uint8_t color);
  static void cursorShow();
  static void cursorHide();
};

#endif // USER_SNAKE_SNAKESCREEN_H
