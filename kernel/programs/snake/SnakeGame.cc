#include <cstring>

#include <programs/snake/SnakeGame.h>

namespace {

using SG = SnakeGame;

constexpr uint8_t COLOR_SNAKE = 0x0A;    // green on black
constexpr uint8_t COLOR_FOOD = 0x0C;     // red on black
constexpr uint8_t COLOR_WALL = 0x07;     // gray on black
constexpr uint8_t COLOR_STATUS = 0x0F;   // white on black
constexpr uint8_t COLOR_GAMEOVER = 0x04; // red on black

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
constexpr int STATUS_LABEL_COLS = 7; // "Score: " takes up 7 chars.
constexpr int STATUS_LABEL_START = 1;
constexpr int SCORE_START_COL = RIGHT_WALL - STATUS_LABEL_COLS;

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

bool SnakeGame::step()
{
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

  if (wallCollision(next)) {
    return false;
  }
  if (selfCollision(next)) {
    return false;
  }

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
  if (curHead.row != next.row || curHead.col != next.col) {
    drawAt(curHead, CHAR_BODY, COLOR_SNAKE);
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

  drawAt(body_[head_], CHAR_HEAD, COLOR_SNAKE);
  drawAt(food_, CHAR_FOOD, COLOR_FOOD);
  drawStatus();
}

void SnakeGame::drawGameOver() const
{
  constexpr const char msg[] = " GAME OVER ";
  const int msgLen = strlen(msg);
  const int col = (TOTAL_COLS - msgLen) / 2;
  for (int i = 0; i < msgLen; ++i) {
    drawAt({CENTER_ROW, col + i}, msg[i], COLOR_GAMEOVER);
  }
}

int SnakeGame::score() const
{
  return score_;
}

bool SnakeGame::wallCollision(Pos p) const
{
  return p.row < TOP_WALL || p.row > BOTTOM_WALL || p.col < LEFT_WALL || p.col > RIGHT_WALL;
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
  Screen::put(STATUS_ROW, 1, 'S', COLOR_STATUS);
  Screen::put(STATUS_ROW, 2, 'c', COLOR_STATUS);
  Screen::put(STATUS_ROW, 3, 'o', COLOR_STATUS);
  Screen::put(STATUS_ROW, 4, 'r', COLOR_STATUS);
  Screen::put(STATUS_ROW, 5, 'e', COLOR_STATUS);
  Screen::put(STATUS_ROW, 6, ':', COLOR_STATUS);
  Screen::put(STATUS_ROW, 7, ' ', COLOR_STATUS);

  int s = score_;
  int pos = SCORE_START_COL;
  if (s == 0) {
    Screen::put(STATUS_ROW, pos, '0', COLOR_STATUS);
  }
  else {
    while (s > 0 && pos > STATUS_LABEL_START + STATUS_LABEL_COLS - 1) {
      Screen::put(STATUS_ROW, pos, static_cast<char>('0' + (s % 10)), COLOR_STATUS);
      s /= 10;
      pos--;
    }
  }
}
