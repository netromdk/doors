#include "SnakeFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SnakeFixture, "boost: not active initially")
{
  g.init(0, true);
  CHECK_FALSE(g.boostActive());
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: zones placed at init")
{
  CHECK(g.boostZoneCount() > 0);
  CHECK(g.boostZoneCount() <= SnakeGame::MAX_BOOST_ZONES);
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: zones within playable bounds")
{
  for (int i = 0; i < g.boostZoneCount(); ++i) {
    auto p = g.boostZonePos(i);
    CHECK(p.row >= 1);
    CHECK(p.row <= SnakeGame::BOARD_ROWS);
    CHECK(p.col >= 1);
    CHECK(p.col <= SnakeGame::BOARD_COLS);
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: zones avoid obstacles")
{
  g.init(0, true);
  for (int bi = 0; bi < g.boostZoneCount(); ++bi) {
    auto bp = g.boostZonePos(bi);
    for (int oi = 0; oi < g.obstacleCount(); ++oi) {
      auto op = g.obstaclePos(oi);
      const bool sameCell = (bp.row == op.row && bp.col == op.col);
      CHECK_FALSE(sameCell);
    }
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: zones avoid snake head")
{
  g.init(0, true);
  for (int i = 0; i < g.boostZoneCount(); ++i) {
    auto p = g.boostZonePos(i);
    const bool atHead = (p.row == 12 && p.col == 40);
    CHECK_FALSE(atHead);
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: consuming a zone activates boost")
{
  auto zone = g.boostZonePos(0);

  // Step once: head moves from (12, 40) to (12, 41).
  g.step(0);

  if (!g.boostActive()) {
    int r = 12;
    int c = 41;
    // Navigate vertically to the target row.
    if (zone.row < r) {
      g.setDir(SnakeGame::Dir::Up);
      while (r > zone.row && g.step(0)) {
        --r;
      }
    }
    else if (zone.row > r) {
      g.setDir(SnakeGame::Dir::Down);
      while (r < zone.row && g.step(0)) {
        ++r;
      }
    }

    // Navigate horizontally to the target column.
    if (zone.col < c) {
      g.setDir(SnakeGame::Dir::Left);
      while (c > zone.col && g.step(0)) {
        --c;
      }
    }
    else if (zone.col > c) {
      g.setDir(SnakeGame::Dir::Right);
      while (c < zone.col && g.step(0)) {
        ++c;
      }
    }
  }

  CHECK(g.boostActive());
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: zone count decreases after consumption")
{
  const int initialCount = g.boostZoneCount();
  REQUIRE(initialCount > 0);

  auto zone = g.boostZonePos(0);
  g.step(0);

  if (!g.boostActive()) {
    int r = 12;
    int c = 41;
    if (zone.row < r) {
      g.setDir(SnakeGame::Dir::Up);
      while (r > zone.row && g.step(0)) {
        --r;
      }
    }
    else if (zone.row > r) {
      g.setDir(SnakeGame::Dir::Down);
      while (r < zone.row && g.step(0)) {
        ++r;
      }
    }
    if (zone.col < c) {
      g.setDir(SnakeGame::Dir::Left);
      while (c > zone.col && g.step(0)) {
        --c;
      }
    }
    else if (zone.col > c) {
      g.setDir(SnakeGame::Dir::Right);
      while (c < zone.col && g.step(0)) {
        ++c;
      }
    }
  }

  if (g.boostActive()) {
    CHECK(g.boostZoneCount() == initialCount - 1);
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: moveIntervalMs unchanged before any boost")
{
  const int normal = g.moveIntervalMs();

  // Step multiple times without direction changes. Since boost zones are passive (no collision),
  // stepping never activates boost on its own.
  for (int i = 0; i < 5; ++i) {
    if (!g.step(0)) {
      break;
    }
    CHECK(g.moveIntervalMs() == normal);
    if (g.boostActive()) {
      break;
    }
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: moveIntervalMs decreases when boost active")
{
  const int normal = g.moveIntervalMs();

  auto zone = g.boostZonePos(0);
  g.step(0);

  if (!g.boostActive()) {
    int r = 12;
    int c = 41;
    if (zone.row < r) {
      g.setDir(SnakeGame::Dir::Up);
      while (r > zone.row && g.step(0)) {
        --r;
      }
    }
    else if (zone.row > r) {
      g.setDir(SnakeGame::Dir::Down);
      while (r < zone.row && g.step(0)) {
        ++r;
      }
    }
    if (zone.col < c) {
      g.setDir(SnakeGame::Dir::Left);
      while (c > zone.col && g.step(0)) {
        --c;
      }
    }
    else if (zone.col > c) {
      g.setDir(SnakeGame::Dir::Right);
      while (c < zone.col && g.step(0)) {
        ++c;
      }
    }
  }

  if (g.boostActive()) {
    CHECK(g.moveIntervalMs() < normal);
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "boost: boost timer counts down with dtMs")
{
  auto zone = g.boostZonePos(0);
  g.step(0);

  if (!g.boostActive()) {
    int r = 12;
    int c = 41;
    if (zone.row < r) {
      g.setDir(SnakeGame::Dir::Up);
      while (r > zone.row && g.step(0)) {
        --r;
      }
    }
    else if (zone.row > r) {
      g.setDir(SnakeGame::Dir::Down);
      while (r < zone.row && g.step(0)) {
        ++r;
      }
    }
    if (zone.col < c) {
      g.setDir(SnakeGame::Dir::Left);
      while (c > zone.col && g.step(0)) {
        --c;
      }
    }
    else if (zone.col > c) {
      g.setDir(SnakeGame::Dir::Right);
      while (c < zone.col && g.step(0)) {
        ++c;
      }
    }
  }

  REQUIRE(g.boostActive());

  // Step with enough dtMs to expire the boost.
  g.step(SnakeGame::BOOST_DURATION_MS);
  CHECK_FALSE(g.boostActive());
}
