#include "SnakeFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SnakeFixture, "food: snake can step without immediate food collision")
{
  // Food was placed somewhere on the board; snake starts at center moving right.  This should not
  // crash.
  CHECK(g.step());
}

TEST_CASE_FIXTURE(SnakeFixture, "food: snake survives many steps regardless of food position")
{
  // Move until the snake hits a wall or self (which will eventually happen on a straight
  // line). This verifies food placement doesn't cause crashes.
  int steps = 0;
  while (g.step() && steps < 200) {
    steps++;
  }

  // Snake should eventually hit the right wall from center (col 40 -> col 79).
  CHECK(steps > 0);
}

TEST_CASE_FIXTURE(SnakeFixture, "food: score does not decrease")
{
  // No matter what happens, score should never go negative.
  int prevScore = g.score();
  for (int i = 0; i < 50; ++i) {
    if (!g.step()) break;
    CHECK(g.score() >= prevScore);
    prevScore = g.score();
  }
}
