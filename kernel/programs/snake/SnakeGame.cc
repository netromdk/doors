#include <cstdio>
#include <cstring>

#include <programs/snake/SnakeGame.h>

namespace {

using SG = SnakeGame;

constexpr uint8_t COLOR_SNAKE = 0x0A;     // green on black
constexpr uint8_t COLOR_SNAKE_ALT = 0x02; // dark green on black
constexpr uint8_t COLOR_FOOD = 0x0C;      // red on black
constexpr uint8_t COLOR_BONUS = 0x0E;     // yellow on black
constexpr uint8_t COLOR_WALL = 0x07;      // gray on black
constexpr uint8_t COLOR_OBSTACLE = 0x08;  // dark grey on black
constexpr uint8_t COLOR_BOOST = 0x0D;     // light magenta on black
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
  char buf[16];
  const int len = snprintf(buf, sizeof(buf), "%d", val);
  for (int i = 0; i < len; ++i) {
    Screen::put(row, col + i, buf[i], color);
  }
  col += len;
}

} // namespace

bool SnakeGame::isOpposite(Dir cur, Dir next)
{
  return (cur == Dir::Up && next == Dir::Down) || (cur == Dir::Down && next == Dir::Up) ||
         (cur == Dir::Left && next == Dir::Right) || (cur == Dir::Right && next == Dir::Left);
}

void SnakeGame::init(uint32_t prngSeed, bool withObstacles)
{
  lcg_ = prngSeed;
  head_ = 0;
  length_ = 1;
  dir_ = Dir::Right;
  score_ = 0;
  body_[0] = {CENTER_ROW, CENTER_COL};
  bonusActive_ = false;
  bonusElapsedMs_ = 0;
  bonusRemainingMs_ = 0;
  scoreHighlightRemainingMs_ = 0;
  obstacleCount_ = 0;
  eatsSinceObstacle_ = 0;
  boostZoneCount_ = 0;
  boostActive_ = false;
  boostTimerMs_ = 0;
  boostCooldownMs_ = 0;
  placeFood();
  if (withObstacles) {
    placeObstacles();
  }
  placeBoostZones();
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

bool SnakeGame::step(uint64_t dtMs)
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
  if (obstacleCollision(next)) {
    return false;
  }

  const int oldHeadIdx = head_;
  const int oldTailIdx = (head_ - length_ + 1 + SNAKE_MAX) % SNAKE_MAX;
  const Pos oldTailPos = body_[oldTailIdx];

  head_ = (head_ + 1) % SNAKE_MAX;
  body_[head_] = next;

  const bool ate = (next.row == food_.row && next.col == food_.col);
  const bool ateBonus = bonusActive_ && (next.row == bonusFood_.row && next.col == bonusFood_.col);

  if (ate) {
    length_++;
    score_++;
    if (obstacleCount_ < MAX_OBSTACLES) {
      eatsSinceObstacle_++;
      if (eatsSinceObstacle_ >= OBSTACLE_STEP_INTERVAL) {
        eatsSinceObstacle_ = 0;
        spawnObstacle();
      }
    }
  }
  else {
    eraseAt(oldTailPos);
  }

  if (ateBonus) {
    score_ += BONUS_POINTS;
    bonusActive_ = false;
    scoreHighlightRemainingMs_ = BONUS_HIGHLIGHT_MS;
    eraseAt(bonusFood_);
  }

  // Boost zone consumption.
  for (int i = 0; i < boostZoneCount_; ++i) {
    if (next.row == boostZones_[i].row && next.col == boostZones_[i].col) {
      boostZones_[i] = boostZones_[--boostZoneCount_];
      boostActive_ = true;
      boostTimerMs_ = BOOST_DURATION_MS;
      break;
    }
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

  // Boost timer.
  if (boostActive_) {
    if (dtMs >= boostTimerMs_) {
      boostActive_ = false;
      boostTimerMs_ = 0;
    }
    else {
      boostTimerMs_ -= dtMs;
    }
  }

  // Boost zone respawn.
  if (boostZoneCount_ == 0) {
    boostCooldownMs_ += dtMs;
    if (boostCooldownMs_ >= BOOST_RESPAWN_MS) {
      placeBoostZones();
      boostCooldownMs_ = 0;
      for (int i = 0; i < boostZoneCount_; ++i) {
        drawAt(boostZones_[i], CHAR_BOOST, COLOR_BOOST);
      }
    }
  }

  // Bonus food timer.
  if (bonusActive_) {
    if (dtMs >= bonusRemainingMs_) {
      eraseAt(bonusFood_);
      bonusActive_ = false;
      bonusElapsedMs_ += dtMs - bonusRemainingMs_;
    }
    else {
      bonusRemainingMs_ -= dtMs;
      bonusElapsedMs_ += dtMs;
    }
  }
  else {
    bonusElapsedMs_ += dtMs;
  }

  // Score highlight countdown.
  if (scoreHighlightRemainingMs_ > 0) {
    if (dtMs >= scoreHighlightRemainingMs_) {
      scoreHighlightRemainingMs_ = 0;
    }
    else {
      scoreHighlightRemainingMs_ -= dtMs;
    }
  }

  if (!bonusActive_ && bonusElapsedMs_ >= BONUS_INTERVAL_MS) {
    bonusElapsedMs_ = 0;
    placeBonusFood();
    bonusRemainingMs_ = bonusDurationMs();
    bonusActive_ = true;
    drawAt(bonusFood_, CHAR_BONUS, COLOR_BONUS);
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
  if (bonusActive_) {
    drawAt(bonusFood_, CHAR_BONUS, COLOR_BONUS);
  }
  for (int i = 0; i < obstacleCount_; ++i) {
    drawAt(obstacles_[i], CHAR_OBSTACLE, COLOR_OBSTACLE);
  }
  for (int i = 0; i < boostZoneCount_; ++i) {
    drawAt(boostZones_[i], CHAR_BOOST, COLOR_BOOST);
  }
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
  constexpr uint8_t COLOR_CD = 0x0E;
  char buf[4];
  snprintf(buf, sizeof(buf), " %d ", n);
  const int col = (TOTAL_COLS - 3) / 2;
  for (int i = 0; i < 3; ++i) {
    drawAt({CENTER_ROW, col + i}, buf[i], COLOR_CD);
  }
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
  char buf[64];
  int len = snprintf(buf, sizeof(buf), "Score: %d  Best: %d", score_, highScore_);
  int c = (TOTAL_COLS - len) / 2;
  drawStr(scoreRow, c, buf, COLOR_SCORE);

  // Quit message at bottom row.
  constexpr const char quitMsg[] = "Press q to quit";
  const int quitMsgLen = strlen(quitMsg);
  const int qc = (TOTAL_COLS - quitMsgLen) / 2;
  for (int i = 0; i < quitMsgLen; ++i) {
    drawAt({BOARD_ROWS + 1, qc + i}, quitMsg[i], COLOR_SCORE);
  }
}

int SnakeGame::score() const
{
  return score_;
}

bool SnakeGame::bonusActive() const
{
  return bonusActive_;
}

SnakeGame::Pos SnakeGame::bonusPos() const
{
  return bonusFood_;
}

int SnakeGame::obstacleCount() const
{
  return obstacleCount_;
}

SnakeGame::Pos SnakeGame::obstaclePos(int i) const
{
  if (i >= obstacleCount_) {
    __builtin_trap();
  }
  return obstacles_[i];
}

bool SnakeGame::boostActive() const
{
  return boostActive_;
}

int SnakeGame::boostZoneCount() const
{
  return boostZoneCount_;
}

SnakeGame::Pos SnakeGame::boostZonePos(int i) const
{
  if (i >= boostZoneCount_) {
    __builtin_trap();
  }
  return boostZones_[i];
}

int SnakeGame::baseIntervalMs() const
{
  const int ms = 200 - length_ * 8;
  return ms < 60 ? 60 : ms;
}

int SnakeGame::moveIntervalMs() const
{
  // Make vertical movement slightly slower than horizontal. This is needed because there are fewer
  // vertical spaces.
  int ms = baseIntervalMs();
  if (boostActive_) {
    ms /= 2;
  }
  if (dir_ == Dir::Up || dir_ == Dir::Down) {
    return (ms * 3 + 1) / 2;
  }
  return ms;
}

uint64_t SnakeGame::bonusDurationMs() const
{
  const uint64_t ms = BONUS_INTERVAL_MS - static_cast<uint64_t>(length_) * 200;
  return ms < BONUS_DURATION_MIN_MS ? BONUS_DURATION_MIN_MS : ms;
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

bool SnakeGame::obstacleCollision(Pos p) const
{
  for (int i = 0; i < obstacleCount_; ++i) {
    if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
      return true;
    }
  }
  return false;
}

void SnakeGame::placeObstacles()
{
  const int maxCells = BOARD_ROWS * BOARD_COLS;
  obstacleCount_ = 0;
  for (int attempt = 0; attempt < maxCells * 3 && obstacleCount_ < OBSTACLE_INIT_COUNT; ++attempt) {
    const int r = static_cast<int>(lcgNext() % BOARD_ROWS) + 1;
    const int c = static_cast<int>(lcgNext() % BOARD_COLS) + 1;
    const Pos p{r, c};

    // Skip snake head.
    if (p.row == body_[head_].row && p.col == body_[head_].col) {
      continue;
    }

    // Skip existing obstacles.
    bool dup = false;
    for (int i = 0; i < obstacleCount_; ++i) {
      if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
        dup = true;
        break;
      }
    }
    if (dup) {
      continue;
    }

    obstacles_[obstacleCount_++] = p;
  }
}

void SnakeGame::spawnObstacle()
{
  const int maxCells = BOARD_ROWS * BOARD_COLS;
  for (int attempt = 0; attempt < maxCells * 2; ++attempt) {
    const int r = static_cast<int>(lcgNext() % BOARD_ROWS) + 1;
    const int c = static_cast<int>(lcgNext() % BOARD_COLS) + 1;
    const Pos p{r, c};

    bool occupied = false;

    // Check snake body.
    for (int i = 0; !occupied && i < length_; ++i) {
      const int idx = (head_ - i + SNAKE_MAX) % SNAKE_MAX;
      if (body_[idx].row == p.row && body_[idx].col == p.col) {
        occupied = true;
      }
    }

    // Check food.
    if (!occupied && p.row == food_.row && p.col == food_.col) {
      occupied = true;
    }

    // Check bonus food.
    if (!occupied && bonusActive_ && p.row == bonusFood_.row && p.col == bonusFood_.col) {
      occupied = true;
    }

    // Check existing obstacles.
    for (int i = 0; !occupied && i < obstacleCount_; ++i) {
      if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
        occupied = true;
      }
    }

    if (!occupied) {
      obstacles_[obstacleCount_++] = p;
      drawAt(p, CHAR_OBSTACLE, COLOR_OBSTACLE);
      return;
    }
  }
}

void SnakeGame::placeBoostZones()
{
  const int maxCells = BOARD_ROWS * BOARD_COLS;
  boostZoneCount_ = 0;
  for (int attempt = 0; attempt < maxCells * 3 && boostZoneCount_ < MAX_BOOST_ZONES; ++attempt) {
    const int r = static_cast<int>(lcgNext() % BOARD_ROWS) + 1;
    const int c = static_cast<int>(lcgNext() % BOARD_COLS) + 1;
    const Pos p{r, c};

    // Skip snake body.
    bool onBody = false;
    for (int i = 0; i < length_; ++i) {
      const int idx = (head_ - i + SNAKE_MAX) % SNAKE_MAX;
      if (body_[idx].row == p.row && body_[idx].col == p.col) {
        onBody = true;
        break;
      }
    }
    if (onBody) {
      continue;
    }

    // Skip obstacles.
    bool onObstacle = false;
    for (int i = 0; i < obstacleCount_; ++i) {
      if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
        onObstacle = true;
        break;
      }
    }
    if (onObstacle) {
      continue;
    }

    // Skip food.
    if (p.row == food_.row && p.col == food_.col) {
      continue;
    }

    // Skip bonus food.
    if (bonusActive_ && p.row == bonusFood_.row && p.col == bonusFood_.col) {
      continue;
    }

    // Skip other boost zones.
    bool onZone = false;
    for (int i = 0; i < boostZoneCount_; ++i) {
      if (boostZones_[i].row == p.row && boostZones_[i].col == p.col) {
        onZone = true;
        break;
      }
    }
    if (onZone) {
      continue;
    }

    boostZones_[boostZoneCount_++] = p;
  }
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
    bool onObstacle = false;
    for (int i = 0; !onSnake && i < obstacleCount_; ++i) {
      if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
        onObstacle = true;
        break;
      }
    }
    if (!onSnake && !onObstacle) {
      food_ = p;
      return;
    }
  }
}

void SnakeGame::placeBonusFood()
{
  const int maxCells = BOARD_ROWS * BOARD_COLS;
  for (int attempt = 0; attempt < maxCells * 2; ++attempt) {
    const int r = static_cast<int>(lcgNext() % BOARD_ROWS) + 1;
    const int c = static_cast<int>(lcgNext() % BOARD_COLS) + 1;
    const Pos p{r, c};

    bool occupied = (p.row == food_.row && p.col == food_.col);
    for (int i = 0; !occupied && i < length_; ++i) {
      const int idx = (head_ - i + SNAKE_MAX) % SNAKE_MAX;
      if (body_[idx].row == p.row && body_[idx].col == p.col) {
        occupied = true;
        break;
      }
    }
    for (int i = 0; !occupied && i < obstacleCount_; ++i) {
      if (obstacles_[i].row == p.row && obstacles_[i].col == p.col) {
        occupied = true;
        break;
      }
    }
    if (!occupied) {
      bonusFood_ = p;
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

  const uint8_t scoreColor = scoreHighlightRemainingMs_ > 0 ? COLOR_BONUS : COLOR_STATUS;

  // Show score.
  char scoreBuf[32];
  int scoreLen = snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score_);
  int pos = 1;
  for (int i = 0; i < scoreLen && pos < RIGHT_WALL; ++i) {
    Screen::put(STATUS_ROW, pos++, scoreBuf[i], scoreColor);
  }

  // Speed indicator: base interval (before diagonal adjustment).
  drawStr(STATUS_ROW, pos, "  Speed: ", COLOR_STATUS);
  drawIntR(STATUS_ROW, pos, baseIntervalMs(), COLOR_STATUS);
  drawStr(STATUS_ROW, pos, "ms", COLOR_STATUS);

  // Obstacle spawn countdown.
  if (obstacleCount_ < MAX_OBSTACLES) {
    const int foodsLeft = OBSTACLE_STEP_INTERVAL - eatsSinceObstacle_;
    drawStr(STATUS_ROW, pos, "  Next wall: ", COLOR_STATUS);
    drawIntR(STATUS_ROW, pos, foodsLeft, COLOR_STATUS);
  }

  // Boost indicator (always draw to erase on deactivation).
  drawStr(STATUS_ROW, pos, boostActive_ ? "  BOOST" : "       ",
          boostActive_ ? COLOR_BONUS : COLOR_STATUS);
}
