#include <cstring>

#include <programs/snake/SnakeGame.h>

namespace {

using SG = SnakeGame;

constexpr uint8_t COLOR_SNAKE = 0x0A;     // green on black
constexpr uint8_t COLOR_SNAKE_ALT = 0x02; // dark green on black
constexpr uint8_t COLOR_FOOD = 0x0C;      // red on black
constexpr uint8_t COLOR_WALL = 0x07;      // gray on black
constexpr uint8_t COLOR_STATUS = 0x0F;    // white on black
constexpr uint8_t COLOR_GAMEOVER = 0x04;  // red on black

// Board wall positions derived from playable-area constants.
constexpr int TOP_WALL = 0;
constexpr int BOTTOM_WALL = SG::BOARD_ROWS + 1;
constexpr int LEFT_WALL = 0;
constexpr int RIGHT_WALL = SG::BOARD_COLS + 1;
constexpr int TOTAL_COLS = SG::BOARD_COLS + 2;

// Center of the playable area (row 1..BOARD_ROWS, col 1..BOARD_COLS).
constexpr int CENTER_ROW = 1 + SG::BOARD_ROWS / 2;
constexpr int CENTER_COL = 1 + SG::BOARD_COLS / 2;

// Status bar layout constants.
constexpr int STATUS_ROW = TOP_WALL;

int highScore_ = 0;

// Draw a null-terminated string left-to-right at (row, col), advancing col.
void drawStr(int row, int &col, const char *s, uint8_t color)
{
  while (*s) {
    Screen::put(row, col++, *s++, color);
  }
}

// Draw val right-aligned at (row, col) and advance col past it.
void drawIntR(int row, int &col, int val, uint8_t color)
{
  int tmp = val;
  int digits = 0;
  do {
    digits++;
    tmp /= 10;
  } while (tmp > 0);
  int pos = col + digits - 1;
  tmp = val;
  do {
    Screen::put(row, pos--, '0' + (tmp % 10), color);
    tmp /= 10;
  } while (tmp > 0);
  col += digits;
}

} // namespace

bool SnakeGame::isOpposite(Dir cur, Dir next)
{
  return (cur == Dir::Up && next == Dir::Down) || (cur == Dir::Down && next == Dir::Up) ||
         (cur == Dir::Left && next == Dir::Right) || (cur == Dir::Right && next == Dir::Left);
}

void SnakeGame::init(uint32_t prngSeed)
{
  lcg_ = prngSeed;
  head_ = 0;
  length_ = 1;
  dir_ = Dir::Right;
  score_ = 0;
  body_[0] = {CENTER_ROW, CENTER_COL};
  placeFood();
}

void SnakeGame::setDir(Dir d)
{
  if (!isOpposite(dir_, d)) {
    dir_ = d;
  }
}

void SnakeGame::setWrapMode(bool on)
{
  wrapMode_ = on;
}

bool SnakeGame::step()
{
  started_ = true;
  const Pos curHead = body_[head_];
  Pos next{};
  switch (dir_) {
  case Dir::Up:
    next = {curHead.row - 1, curHead.col};
    break;
  case Dir::Down:
    next = {curHead.row + 1, curHead.col};
    break;
  case Dir::Left:
    next = {curHead.row, curHead.col - 1};
    break;
  case Dir::Right:
    next = {curHead.row, curHead.col + 1};
    break;
  }

  if (wrapMode_) {
    if (next.row < 1) {
      next.row = BOARD_ROWS;
    }
    else if (next.row > BOARD_ROWS) {
      next.row = 1;
    }
    if (next.col < 1) {
      next.col = BOARD_COLS;
    }
    else if (next.col > BOARD_COLS) {
      next.col = 1;
    }
  }
  else if (wallCollision(next)) {
    return false;
  }
  if (selfCollision(next)) {
    return false;
  }

  const int oldHeadIdx = head_;
  const int oldTailIdx = (head_ - length_ + 1 + SNAKE_MAX) % SNAKE_MAX;
  const Pos oldTailPos = body_[oldTailIdx];

  head_ = (head_ + 1) % SNAKE_MAX;
  body_[head_] = next;

  const bool ate = (next.row == food_.row && next.col == food_.col);
  if (ate) {
    length_++;
    score_++;
  }
  else {
    eraseAt(oldTailPos);
  }

  drawAt(next, CHAR_HEAD, COLOR_SNAKE);
  if (length_ > 1) {
    drawAt(curHead, CHAR_BODY, oldHeadIdx % 2 == 0 ? COLOR_SNAKE : COLOR_SNAKE_ALT);
  }
  drawStatus();

  if (ate) {
    placeFood();
    drawAt(food_, CHAR_FOOD, COLOR_FOOD);
  }

  return true;
}

void SnakeGame::drawBoard() const
{
  if (wrapMode_) {
    // Erase cosmetic walls. In wrap mode the border is empty and the initial classic-mode wall
    // artifacts are cleaned up here.
    for (int c = 0; c <= RIGHT_WALL; ++c) {
      eraseAt({TOP_WALL, c});
      eraseAt({BOTTOM_WALL, c});
    }
    for (int r = 1; r < BOTTOM_WALL; ++r) {
      eraseAt({r, LEFT_WALL});
      eraseAt({r, RIGHT_WALL});
    }
  }
  else {
    // Corners.
    drawAt({TOP_WALL, LEFT_WALL}, CHAR_CORNER, COLOR_WALL);
    drawAt({TOP_WALL, RIGHT_WALL}, CHAR_CORNER, COLOR_WALL);
    drawAt({BOTTOM_WALL, LEFT_WALL}, CHAR_CORNER, COLOR_WALL);
    drawAt({BOTTOM_WALL, RIGHT_WALL}, CHAR_CORNER, COLOR_WALL);

    // Top and bottom walls.
    for (int c = 1; c < RIGHT_WALL; ++c) {
      drawAt({TOP_WALL, c}, CHAR_HWALL, COLOR_WALL);
      drawAt({BOTTOM_WALL, c}, CHAR_HWALL, COLOR_WALL);
    }

    // Left and right walls.
    for (int r = 1; r < BOTTOM_WALL; ++r) {
      drawAt({r, LEFT_WALL}, CHAR_VWALL, COLOR_WALL);
      drawAt({r, RIGHT_WALL}, CHAR_VWALL, COLOR_WALL);
    }
  }

  drawAt(body_[head_], CHAR_HEAD, COLOR_SNAKE);
  drawAt(food_, CHAR_FOOD, COLOR_FOOD);
  drawStatus();
}

void SnakeGame::drawPause() const
{
  constexpr const char msg[] = " PAUSED ";
  constexpr uint8_t COLOR_PAUSE = 0x0E; // yellow on black
  const int msgLen = strlen(msg);
  const int col = (TOTAL_COLS - msgLen) / 2;
  for (int i = 0; i < msgLen; ++i) {
    drawAt({CENTER_ROW, col + i}, msg[i], COLOR_PAUSE);
  }
}

void SnakeGame::drawCountdown(int n) const
{
  constexpr uint8_t COLOR_CD = 0x0E; // yellow on black
  const int col = (TOTAL_COLS - 3) / 2;
  drawAt({CENTER_ROW, col}, ' ', COLOR_CD);
  drawAt({CENTER_ROW, col + 1}, static_cast<char>('0' + n), COLOR_CD);
  drawAt({CENTER_ROW, col + 2}, ' ', COLOR_CD);
}

void SnakeGame::clearOverlay() const
{
  for (int c = 1; c < RIGHT_WALL; ++c) {
    eraseAt({CENTER_ROW, c});
  }
}

void SnakeGame::drawGameOver() const
{
  constexpr const char msg[] = " GAME OVER ";
  const int msgLen = strlen(msg);
  const int col = (TOTAL_COLS - msgLen) / 2;
  for (int i = 0; i < msgLen; ++i) {
    drawAt({CENTER_ROW, col + i}, msg[i], COLOR_GAMEOVER);
  }

  // Second line: "Score: N  Best: N" centered on CENTER_ROW + 1
  const int scoreRow = CENTER_ROW + 1;
  constexpr uint8_t COLOR_SCORE = 0x0F; // white on black

  int tmp = score_;
  int sd = 0;
  do {
    sd++;
    tmp /= 10;
  } while (tmp > 0);

  tmp = highScore_;
  int hd = 0;
  do {
    hd++;
    tmp /= 10;
  } while (tmp > 0);

  constexpr int SLEN = 7; // "Score: "
  constexpr int BLEN = 7; // "  Best: "
  const int total = SLEN + sd + BLEN + hd;
  int c = (TOTAL_COLS - total) / 2;

  drawStr(scoreRow, c, "Score: ", COLOR_SCORE);
  drawIntR(scoreRow, c, score_, COLOR_SCORE);
  drawStr(scoreRow, c, "  Best: ", COLOR_SCORE);
  drawIntR(scoreRow, c, highScore_, COLOR_SCORE);
}

int SnakeGame::score() const
{
  return score_;
}

int SnakeGame::moveIntervalMs() const
{
  const int ms = 200 - length_ * 8;
  const int clamped = ms < 60 ? 60 : ms;

  // Make vertical movement slightly slower than horizontal. This is needed because there are fewer
  // vertical spaces.
  if (dir_ == Dir::Up || dir_ == Dir::Down) {
    return (clamped * 3 + 1) / 2;
  }

  return clamped;
}

int SnakeGame::highScore()
{
  return highScore_;
}

void SnakeGame::setHighScore(int s)
{
  highScore_ = s;
}

bool SnakeGame::wallCollision(Pos p) const
{
  return p.row < 1 || p.row > BOARD_ROWS || p.col < 1 || p.col > BOARD_COLS;
}

bool SnakeGame::selfCollision(Pos p) const
{
  for (int i = 1; i < length_; ++i) {
    const int idx = (head_ - i + SNAKE_MAX) % SNAKE_MAX;
    if (body_[idx].row == p.row && body_[idx].col == p.col) {
      return true;
    }
  }
  return false;
}

void SnakeGame::placeFood()
{
  const int maxCells = BOARD_ROWS * BOARD_COLS;
  for (int attempt = 0; attempt < maxCells * 2; ++attempt) {
    const int r = static_cast<int>(lcgNext() % BOARD_ROWS) + 1;
    const int c = static_cast<int>(lcgNext() % BOARD_COLS) + 1;
    const Pos p{r, c};

    bool onSnake = false;
    for (int i = 0; i < length_; ++i) {
      const int idx = (head_ - i + SNAKE_MAX) % SNAKE_MAX;
      if (body_[idx].row == p.row && body_[idx].col == p.col) {
        onSnake = true;
        break;
      }
    }
    if (!onSnake) {
      food_ = p;
      return;
    }
  }
}

uint32_t SnakeGame::lcgNext()
{
  // Linear Congruential Generator (LCG): x_{n+1} = (a * x_n + c) mod 2^32.
  // Parameters from Knuth's MMIX (hypothetical CPU in The Art of Computer Programming).
  lcg_ = lcg_ * 1664525u + 1013904223u;
  return lcg_;
}

void SnakeGame::eraseAt(Pos p) const
{
  Screen::put(p.row, p.col, ' ', 0);
}

void SnakeGame::drawAt(Pos p, char ch, uint8_t color) const
{
  Screen::put(p.row, p.col, ch, color);
}

void SnakeGame::drawStatus() const
{
  constexpr uint8_t COLOR_MODE = 0x0E; // yellow on black

  if (started_) {
    const char *modeStr = wrapMode_ ? "Wrap" : "Classic";
    int modeLen = strlen(modeStr);
    int modeCol = RIGHT_WALL - modeLen;
    for (int i = 0; i < modeLen; ++i) {
      Screen::put(STATUS_ROW, modeCol + i, modeStr[i], COLOR_MODE);
    }
  }

  // "Score: " at cols 1-7.
  Screen::put(STATUS_ROW, 1, 'S', COLOR_STATUS);
  Screen::put(STATUS_ROW, 2, 'c', COLOR_STATUS);
  Screen::put(STATUS_ROW, 3, 'o', COLOR_STATUS);
  Screen::put(STATUS_ROW, 4, 'r', COLOR_STATUS);
  Screen::put(STATUS_ROW, 5, 'e', COLOR_STATUS);
  Screen::put(STATUS_ROW, 6, ':', COLOR_STATUS);
  Screen::put(STATUS_ROW, 7, ' ', COLOR_STATUS);

  // Score digits immediately after, left-to-right.
  int s = score_;
  int pos = 8;
  if (s == 0) {
    Screen::put(STATUS_ROW, pos, '0', COLOR_STATUS);
  }
  else {
    int div = 1;
    while (div * 10 <= s) {
      div *= 10;
    }
    while (div > 0) {
      Screen::put(STATUS_ROW, pos++, '0' + (s / div) % 10, COLOR_STATUS);
      div /= 10;
    }
  }
}
