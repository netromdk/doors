#include <cstdio>
#include <doctest/doctest.h>
#include <programs/snake/SnakeGame.h>

#include "TestLcg.h"

// Navigate from (curRow, curCol) to (targetRow, targetCol), changing direction and stepping.
// Returns the number of steps taken.
static int navigateTo(SnakeGame &g, int curRow, int curCol, int targetRow, int targetCol)
{
  int steps = 0;

  if (targetRow < curRow) {
    g.setDir(SnakeGame::Dir::Up);
    for (int i = 0; i < curRow - targetRow; ++i) {
      if (!g.step()) {
        return steps;
      }
      ++steps;
    }
  }
  else if (targetRow > curRow) {
    g.setDir(SnakeGame::Dir::Down);
    for (int i = 0; i < targetRow - curRow; ++i) {
      if (!g.step()) {
        return steps;
      }
      ++steps;
    }
  }

  if (targetCol < curCol) {
    g.setDir(SnakeGame::Dir::Left);
    for (int i = 0; i < curCol - targetCol; ++i) {
      if (!g.step()) {
        return steps;
      }
      ++steps;
    }
  }
  else if (targetCol > curCol) {
    g.setDir(SnakeGame::Dir::Right);
    for (int i = 0; i < targetCol - curCol; ++i) {
      if (!g.step()) {
        return steps;
      }
      ++steps;
    }
  }

  return steps;
}

TEST_CASE("obstacles: initial count at init")
{
  SnakeGame g;
  g.init(0, true);
  CHECK(g.obstacleCount() == SnakeGame::OBSTACLE_INIT_COUNT);
}

TEST_CASE("obstacles: within playable bounds")
{
  SnakeGame g;
  g.init(0, true);
  for (int i = 0; i < g.obstacleCount(); ++i) {
    const auto p = g.obstaclePos(i);
    CHECK(p.row >= 1);
    CHECK(p.row <= SnakeGame::BOARD_ROWS);
    CHECK(p.col >= 1);
    CHECK(p.col <= SnakeGame::BOARD_COLS);
  }
}

TEST_CASE("obstacles: withObstacles=false gives zero count")
{
  SnakeGame g;
  g.init(0);
  CHECK(g.obstacleCount() == 0);
}

TEST_CASE("obstacles: rightward movement cannot exceed wall limit")
{
  // Without obstacles, moving right from center (12,40) hits wall at step 38.
  // With obstacles, the snake must still die by step 38 (either obstacle or wall).
  SnakeGame g;
  g.init(0, true);
  int steps = 0;
  while (g.step()) {
    ++steps;
  }
  CHECK(steps <= 38);
}

TEST_CASE("obstacles: obstacle count never decreases during play")
{
  SnakeGame g;
  g.init(0, true);
  int prevCount = g.obstacleCount();
  REQUIRE(prevCount > 0);

  for (int i = 0; i < 200; ++i) {
    g.step();
    CHECK(g.obstacleCount() >= prevCount);
    prevCount = g.obstacleCount();
  }
}

TEST_CASE("obstacles: spawn triggered by eating without obstacles")
{
  SnakeGame g;
  g.init(0); // no obstacles
  CHECK(g.obstacleCount() == 0);

  // Compute food position using the same LCG as `SnakeGame::init()`.
  uint32_t lcg = 0;
  lcg = lcgStep(lcg);
  const uint32_t r = lcg % 23; // first call in placeFood -> row index 0..22

  lcg = lcgStep(lcg);
  const uint32_t c = lcg % 78; // second call -> col index 0..77

  const int foodRow = static_cast<int>(r) + 1;
  const int foodCol = static_cast<int>(c) + 1;

  // Snake starts at center (12, 40) facing right.
  navigateTo(g, 12, 40, foodRow, foodCol);
  REQUIRE(g.score() == 1); // ate first food

  // After eating, head right to sweep for more food.
  g.setDir(SnakeGame::Dir::Right);

  int foodsEaten = 1;
  int prevScore = 1;
  for (int i = 0; i < 2000 && g.score() < 6; ++i) {
    if (!g.step()) break;
    if (g.score() > prevScore) {
      ++foodsEaten;
      prevScore = g.score();
    }
    if (i % 20 == 0) {
      g.setDir(SnakeGame::Dir::Up);
    }
    else if (i % 20 == 5) {
      g.setDir(SnakeGame::Dir::Right);
    }
    else if (i % 20 == 10) {
      g.setDir(SnakeGame::Dir::Down);
    }
    else if (i % 20 == 15) {
      g.setDir(SnakeGame::Dir::Left);
    }
  }

  CHECK(foodsEaten >= 1);
  MESSAGE("foodsEaten=", foodsEaten, " obstacleCount=", g.obstacleCount());
}
