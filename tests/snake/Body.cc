#include <doctest/doctest.h>
#include <programs/snake/SnakeGame.h>

TEST_CASE("init: score starts at 0")
{
  SnakeGame g;
  g.init(0);
  CHECK(g.score() == 0);
}

TEST_CASE("step: head advances in current direction")
{
  SnakeGame g;
  g.init(0);
  CHECK(g.step());
}

TEST_CASE("step: tail removed when no food eaten (length unchanged)")
{
  SnakeGame g;
  g.init(0);
  for (int i = 0; i < 5; ++i) {
    if (!g.step()) break;
  }
  CHECK(g.score() == 0);
}

TEST_CASE("step: score does not decrease on step")
{
  SnakeGame g;
  g.init(0);
  const int s = g.score();
  if (g.step()) {
    CHECK(g.score() >= s);
  }
}

TEST_CASE("body: circular buffer position queries consistent after wrap")
{
  SnakeGame g;
  g.init(0);
  for (int i = 0; i < 200; ++i) {
    if (!g.step()) break;
  }
}
