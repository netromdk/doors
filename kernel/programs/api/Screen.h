#ifndef PROGRAMS_API_SCREEN_H
#define PROGRAMS_API_SCREEN_H

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

#endif // PROGRAMS_API_SCREEN_H
