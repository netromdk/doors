#include "SnakeFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SnakeFixture, "no bonus when dtMs is 0")
{
  for (int i = 0; i < 100; ++i) {
    g.step(0);
  }
  CHECK_FALSE(g.bonusActive());
}

TEST_CASE_FIXTURE(SnakeFixture, "bonus spawns after BONUS_INTERVAL_MS elapses")
{
  CHECK_FALSE(g.bonusActive());
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  CHECK(g.bonusActive());
}

TEST_CASE_FIXTURE(SnakeFixture, "bonus despawns after its duration elapses")
{
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  REQUIRE(g.bonusActive());
  g.step(10000);
  CHECK_FALSE(g.bonusActive());
}

TEST_CASE_FIXTURE(SnakeFixture, "bonus position is inside playable area")
{
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  REQUIRE(g.bonusActive());
  auto p = g.bonusPos();
  CHECK(p.row >= 1);
  CHECK(p.row <= SnakeGame::BOARD_ROWS);
  CHECK(p.col >= 1);
  CHECK(p.col <= SnakeGame::BOARD_COLS);
}

TEST_CASE_FIXTURE(SnakeFixture, "score unchanged by bonus lifecycle")
{
  CHECK(g.score() == 0);
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  CHECK(g.score() == 0);
  g.step(10000);
  CHECK(g.score() == 0);
}

TEST_CASE_FIXTURE(SnakeFixture, "eating bonus adds BONUS_POINTS")
{
  // Spawn bonus.
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  REQUIRE(g.bonusActive());
  auto bonus = g.bonusPos();

  // Step moved head from (12, 40) right to (12, 41). Track it.
  int r = 12, c = 41;

  // Navigate vertically to bonus row.
  auto moveVertical = [&](int target) {
    while (r != target) {
      if (target < r) {
        g.setDir(SnakeGame::Dir::Up);
        --r;
      }
      else {
        g.setDir(SnakeGame::Dir::Down);
        ++r;
      }
      g.step(0);
    }
  };

  // Navigate horizontally to bonus col.
  auto moveHorizontal = [&](int target) {
    while (c != target) {
      if (target < c) {
        g.setDir(SnakeGame::Dir::Left);
        --c;
      }
      else {
        g.setDir(SnakeGame::Dir::Right);
        ++c;
      }
      g.step(0);
    }
  };

  moveVertical(bonus.row);
  moveHorizontal(bonus.col);

  // Head is now at the bonus cell — should have been eaten.
  CHECK(g.score() >= SnakeGame::BONUS_POINTS);
}

TEST_CASE_FIXTURE(SnakeFixture, "bonus respawns after another cycle")
{
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  REQUIRE(g.bonusActive());
  g.step(10000);
  CHECK_FALSE(g.bonusActive());

  // Wait for next cycle.
  g.step(SnakeGame::BONUS_INTERVAL_MS);
  CHECK(g.bonusActive());
}
