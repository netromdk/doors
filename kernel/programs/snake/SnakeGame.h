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
  static constexpr char CHAR_BONUS = '$';
  static constexpr char CHAR_OBSTACLE = '#';
  static constexpr char CHAR_HWALL = '-';
  static constexpr char CHAR_VWALL = '|';
  static constexpr char CHAR_CORNER = '+';

  // Bonus food constants.
  static constexpr int BONUS_POINTS = 3;
  static constexpr uint64_t BONUS_INTERVAL_MS = 8000;
  static constexpr uint64_t BONUS_DURATION_MIN_MS = 4000;
  static constexpr uint64_t BONUS_HIGHLIGHT_MS = 2000;

  // Obstacle constants.
  static constexpr int MAX_OBSTACLES = 20;
  static constexpr int OBSTACLE_INIT_COUNT = 10;
  static constexpr int OBSTACLE_STEP_INTERVAL = 3;

  void init(uint32_t prngSeed, bool withObstacles = false);
  void setDir(Dir d);
  void setWrapMode(bool on);
  bool step(uint64_t dtMs = 0);
  void drawBoard() const;
  void drawPause() const;
  void drawCountdown(int n) const;
  void drawGameOver() const;
  void clearOverlay() const;
  int score() const;
  int moveIntervalMs() const;
  bool bonusActive() const;
  Pos bonusPos() const;
  int obstacleCount() const;
  Pos obstaclePos(int i) const;

  static bool isOpposite(Dir cur, Dir next);
  static int highScore();
  static void setHighScore(int s);

private:
  Pos body_[SNAKE_MAX]{};
  int head_{0}, length_{1};
  Dir dir_{Dir::Right};
  Pos food_{};
  Pos bonusFood_{};
  Pos obstacles_[MAX_OBSTACLES]{};
  int obstacleCount_{0};
  int eatsSinceObstacle_{0};
  uint32_t lcg_{0};
  int score_{0};
  uint64_t bonusElapsedMs_{0};
  uint64_t bonusRemainingMs_{0};
  uint64_t scoreHighlightRemainingMs_{0};
  bool wrapMode_{false};
  bool started_{false};
  bool bonusActive_{false};

  bool wallCollision(Pos p) const;
  bool selfCollision(Pos p) const;
  bool obstacleCollision(Pos p) const;
  void placeFood();
  void placeBonusFood();
  void placeObstacles();
  void spawnObstacle();
  uint64_t bonusDurationMs() const;
  int baseIntervalMs() const;
  uint32_t lcgNext();

  void eraseAt(Pos p) const;
  void drawAt(Pos p, char ch, uint8_t color) const;
  void drawStatus() const;
};

#endif // PROGRAMS_SNAKE_SNAKEGAME_H
