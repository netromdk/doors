#include "SnakeFixture.h"
#include <doctest/doctest.h>

using Dir = SnakeGame::Dir;

TEST_CASE_FIXTURE(SnakeFixture, "isOpposite: Up/Down are opposite")
{
  CHECK(SnakeGame::isOpposite(Dir::Up, Dir::Down));
  CHECK(SnakeGame::isOpposite(Dir::Down, Dir::Up));
}

TEST_CASE_FIXTURE(SnakeFixture, "isOpposite: Left/Right are opposite")
{
  CHECK(SnakeGame::isOpposite(Dir::Left, Dir::Right));
  CHECK(SnakeGame::isOpposite(Dir::Right, Dir::Left));
}

TEST_CASE_FIXTURE(SnakeFixture, "isOpposite: orthogonal pairs are not opposite")
{
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Up, Dir::Left));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Up, Dir::Right));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Down, Dir::Left));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Down, Dir::Right));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Left, Dir::Up));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Left, Dir::Down));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Right, Dir::Up));
  CHECK_FALSE(SnakeGame::isOpposite(Dir::Right, Dir::Down));
}

TEST_CASE_FIXTURE(SnakeFixture, "setDir: ignores directly opposite direction")
{
  g.setDir(Dir::Up);
  g.setDir(Dir::Down);

  // Still moves up (Down was rejected).
  CHECK(g.step());
}

TEST_CASE_FIXTURE(SnakeFixture, "setDir: accepts perpendicular direction")
{
  g.setDir(Dir::Right);
  g.setDir(Dir::Down);

  // Direction changed to Down.
  CHECK(g.step());
}

TEST_CASE_FIXTURE(SnakeFixture, "setDir: accepts same direction (no-op)")
{
  g.setDir(Dir::Right);
  g.setDir(Dir::Right); // no change
  CHECK(g.step());
}
