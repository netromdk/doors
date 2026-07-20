#ifndef TESTS_SNAKE_TESTLCG_H
#define TESTS_SNAKE_TESTLCG_H

#include <cstdint>

inline uint32_t lcgStep(uint32_t state)
{
  return (state * 1664525u) + 1013904223u;
}

#endif // TESTS_SNAKE_TESTLCG_H
