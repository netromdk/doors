#ifndef TESTS_SNAKE_SNAKEFIXTURE_H
#define TESTS_SNAKE_SNAKEFIXTURE_H

#include <doctest/doctest.h>
#include "Game.h"

struct SnakeFixture {
  SnakeGame g;

  SnakeFixture()
  {
    g.init(0);
  }
};

#endif // TESTS_SNAKE_SNAKEFIXTURE_H
