#include <doctest/doctest.h>
#include <programs/snake/SnakeGame.h>

#include "TestLcg.h"

TEST_CASE("lcg: known seed produces known first value")
{
  CHECK(lcgStep(0u) == 1013904223u);
  CHECK(lcgStep(1013904223u) == 1196435762u);
  CHECK(lcgStep(1196435762u) == 3519870697u);
}

TEST_CASE("lcg: sequence is deterministic")
{
  // Two games with same seed should behave identically.
  SnakeGame a;
  a.init(42);

  SnakeGame b;
  b.init(42);

  for (int i = 0; i < 10; ++i) {
    const bool ra = a.step();
    const bool rb = b.step();
    CHECK(ra == rb);
    CHECK(a.score() == b.score());
  }
}

TEST_CASE("lcg: different seeds give deterministic runs")
{
  SnakeGame a;
  a.init(1);

  SnakeGame b;
  b.init(2);

  // Both should run without crashing.
  for (int i = 0; i < 10; ++i) {
    if (!a.step()) break;
    if (!b.step()) break;
  }
}
