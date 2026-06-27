#include <doctest/doctest.h>
#include <programs/snake/SnakeGame.h>

TEST_CASE("wallCollision: interior (1,1) is free")
{
  SnakeGame g;
  g.init(0);

  // Initial position is interior, should be safe.
  CHECK(g.step());
}

TEST_CASE("selfCollision: straight snake does not self-collide")
{
  SnakeGame g;
  g.init(0);

  // Move right many steps. Should not self-collide.
  for (int i = 0; i < 10; ++i) {
    CHECK(g.step());
  }
}

TEST_CASE("wallCollision: right wall at col 79 stops forward movement")
{
  SnakeGame g;
  g.init(0);

  // CENTER_COL = 40, BOARD_COLS = 78.
  // From col 40, moving right reaches col 78 in 38 steps.
  // Step 39 tries col 79 and hits the wall.
  int steps = 0;
  while (g.step()) {
    ++steps;
  }
  CHECK(steps == 38);
}

TEST_CASE("wallCollision: all four directions are safe in interior")
{
  SnakeGame g;
  g.init(0);

  // Box pattern: Right -> Down -> Left -> Up -> Right.
  // All moves stay well inside the playable area.
  g.setDir(SnakeGame::Dir::Down);
  CHECK(g.step());

  g.setDir(SnakeGame::Dir::Left);
  CHECK(g.step());

  g.setDir(SnakeGame::Dir::Up);
  CHECK(g.step());

  g.setDir(SnakeGame::Dir::Right);
  CHECK(g.step());
}

TEST_CASE("wallCollision: bottom-right corner (23,78) is reachable")
{
  SnakeGame g;
  g.init(0);

  // From center (12, 40): go down 11 to row 23, then right 38 to col 78.
  g.setDir(SnakeGame::Dir::Down);
  for (int i = 0; i < 11; ++i) {
    REQUIRE(g.step());
  }

  g.setDir(SnakeGame::Dir::Right);
  for (int i = 0; i < 38; ++i) {
    REQUIRE(g.step());
  }

  // Next step would try col 79 and hit the wall.
  CHECK_FALSE(g.step());
}

TEST_CASE("wallCollision: interior (1,1) is safe after navigation")
{
  SnakeGame g;
  g.init(0);

  // Navigate to the top-left interior corner (1, 1).
  // From (12, 40), go up 11, then left 39.
  g.setDir(SnakeGame::Dir::Up);
  for (int i = 0; i < 11; ++i) {
    REQUIRE(g.step());
  }

  g.setDir(SnakeGame::Dir::Left);
  for (int i = 0; i < 39; ++i) {
    REQUIRE(g.step());
  }

  // Now at (1, 1), a step in any safe direction should still work.
  g.setDir(SnakeGame::Dir::Down);
  CHECK(g.step());
}

TEST_CASE("wallCollision: row 0 is wall (status bar)")
{
  SnakeGame g;
  g.init(0);

  // From center (12, 40), go up 11 steps to row 1.
  g.setDir(SnakeGame::Dir::Up);
  for (int i = 0; i < 11; ++i) {
    REQUIRE(g.step());
  }

  // Next step tries row 0 and hits the wall.
  CHECK_FALSE(g.step());
}

TEST_CASE("wallCollision: row 24 is wall (bottom border)")
{
  SnakeGame g;
  g.init(0);

  // From center (12, 40), go down 11 steps to row 23.
  g.setDir(SnakeGame::Dir::Down);
  for (int i = 0; i < 11; ++i) {
    REQUIRE(g.step());
  }

  // Next step tries row 24 and hits the wall.
  CHECK_FALSE(g.step());
}

TEST_CASE("wallCollision: col 0 is wall")
{
  SnakeGame g;
  g.init(0);

  // From center (12, 40), go down 1, then left 39 to col 1.
  g.setDir(SnakeGame::Dir::Down);
  REQUIRE(g.step());

  g.setDir(SnakeGame::Dir::Left);
  for (int i = 0; i < 39; ++i) {
    REQUIRE(g.step());
  }

  // Next step tries col 0 and hits the wall.
  CHECK_FALSE(g.step());
}

TEST_CASE("wrap: right wall wraps to col 1")
{
  SnakeGame g;
  g.init(0);
  g.setWrapMode(true);

  // Classic mode dies at step 38 (col 79). Wrap mode survives.
  for (int i = 0; i < 50; ++i) {
    CHECK(g.step());
  }
}

TEST_CASE("wrap: left wall wraps to col 78")
{
  SnakeGame g;
  g.init(0);
  g.setWrapMode(true);

  // Move right 1, then left past col 1.
  g.setDir(SnakeGame::Dir::Down);
  REQUIRE(g.step());

  g.setDir(SnakeGame::Dir::Left);
  for (int i = 0; i < 50; ++i) {
    CHECK(g.step());
  }
}

TEST_CASE("wrap: top wall wraps to row 23")
{
  SnakeGame g;
  g.init(0);
  g.setWrapMode(true);

  // Move up past row 1.
  g.setDir(SnakeGame::Dir::Up);
  for (int i = 0; i < 20; ++i) {
    CHECK(g.step());
  }
}

TEST_CASE("wrap: bottom wall wraps to row 1")
{
  SnakeGame g;
  g.init(0);
  g.setWrapMode(true);

  // Move down past row 23.
  g.setDir(SnakeGame::Dir::Down);
  for (int i = 0; i < 20; ++i) {
    CHECK(g.step());
  }
}
