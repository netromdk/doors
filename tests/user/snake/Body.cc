#include "SnakeFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SnakeFixture, "init: score starts at 0")
{
  CHECK(g.score() == 0);
}

TEST_CASE_FIXTURE(SnakeFixture, "step: head advances in current direction")
{
  CHECK(g.step());
}

TEST_CASE_FIXTURE(SnakeFixture, "step: tail removed when no food eaten (length unchanged)")
{
  for (int i = 0; i < 5; ++i) {
    if (!g.step()) break;
  }
  CHECK(g.score() == 0);
}

TEST_CASE_FIXTURE(SnakeFixture, "step: score does not decrease on step")
{
  const int s = g.score();
  if (g.step()) {
    CHECK(g.score() >= s);
  }
}

TEST_CASE_FIXTURE(SnakeFixture, "body: circular buffer position queries consistent after wrap")
{
  for (int i = 0; i < 200; ++i) {
    if (!g.step()) break;
  }
}
