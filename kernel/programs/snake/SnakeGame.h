#ifndef PROGRAMS_SNAKE_SNAKEGAME_H
#define PROGRAMS_SNAKE_SNAKEGAME_H

#include <cstdint>

#include <programs/api/Input.h>
#include <programs/api/Screen.h>

class SnakeGame {
public:
  enum class Dir { Up, Down, Left, Right };
  struct Pos {
    int row, col;
  };

  // Board dimensions:
  //   row 0 = status
  //   rows 1..23 = play
  //   col 0 and 79 = walls
  static constexpr int BOARD_ROWS = 23; // playable rows: 1..23
  static constexpr int BOARD_COLS = 78; // playable cols: 1..78
  static constexpr int SNAKE_MAX = 100;

  // Visual constants.
  static constexpr char CHAR_HEAD = '@';
  static constexpr char CHAR_BODY = 'o';
  static constexpr char CHAR_FOOD = '*';
  static constexpr char CHAR_HWALL = '-';
  static constexpr char CHAR_VWALL = '|';
  static constexpr char CHAR_CORNER = '+';

  void init(uint32_t prngSeed);
  void setDir(Dir d);
  bool step();
  void drawBoard() const;
  void drawPause() const;
  void drawCountdown(int n) const;
  void drawGameOver() const;
  void clearOverlay() const;
  int score() const;

  static bool isOpposite(Dir cur, Dir next);

private:
  Pos body_[SNAKE_MAX]{};
  int head_{0}, length_{1};
  Dir dir_{Dir::Right};
  Pos food_{};
  uint32_t lcg_{0};
  int score_{0};

  bool wallCollision(Pos p) const;
  bool selfCollision(Pos p) const;
  void placeFood();
  uint32_t lcgNext();

  void eraseAt(Pos p) const;
  void drawAt(Pos p, char ch, uint8_t color) const;
  void drawStatus() const;
};

#endif // PROGRAMS_SNAKE_SNAKEGAME_H
