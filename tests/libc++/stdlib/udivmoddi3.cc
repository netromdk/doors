#include <doctest/doctest.h>
#include <stdint.h>

extern "C" uint64_t __udivdi3(uint64_t a, uint64_t b);
extern "C" uint64_t __umoddi3(uint64_t a, uint64_t b);
extern "C" uint64_t __udivmoddi4(uint64_t a, uint64_t b, uint64_t *rem);

TEST_CASE("__udivdi3 and __umoddi3")
{
  CHECK(__udivdi3(10, 3) == 3);
  CHECK(__umoddi3(10, 3) == 1);

  // Divisor > dividend -> quotient 0, remainder = dividend.
  CHECK(__udivdi3(5, 10) == 0);
  CHECK(__umoddi3(5, 10) == 5);

  // Divisor = 1.
  CHECK(__udivdi3(0xDEADBEEFCAFEULL, 1) == 0xDEADBEEFCAFEULL);
  CHECK(__umoddi3(0xDEADBEEFCAFEULL, 1) == 0);

  // Dividend = divisor.
  CHECK(__udivdi3(42, 42) == 1);
  CHECK(__umoddi3(42, 42) == 0);

  // Large dividend, small divisor. Forces shift-and-subtract loop.
  CHECK(__udivdi3(0x100000000ULL, 3) == 0x55555555);
  CHECK(__umoddi3(0x100000000ULL, 3) == 1);

  // Max value/2.
  CHECK(__udivdi3(UINT64_MAX, 2) == 0x7FFFFFFFFFFFFFFFULL);
  CHECK(__umoddi3(UINT64_MAX, 2) == 1);

  // Large divisor near max.
  CHECK(__udivdi3(UINT64_MAX, UINT64_MAX - 1) == 1);
  CHECK(__umoddi3(UINT64_MAX, UINT64_MAX - 1) == 1);

  // Division by zero returns 0.
  CHECK(__udivdi3(42, 0) == 0);
  CHECK(__umoddi3(42, 0) == 0);
}

TEST_CASE("__udivmoddi4")
{
  uint64_t rem;
  CHECK(__udivmoddi4(10, 3, &rem) == 3);
  CHECK(rem == 1);

  // Divisor > dividend.
  CHECK(__udivmoddi4(5, 10, &rem) == 0);
  CHECK(rem == 5);

  // Divisor = 1.
  CHECK(__udivmoddi4(0xDEADBEEFCAFEULL, 1, &rem) == 0xDEADBEEFCAFEULL);
  CHECK(rem == 0);

  // Dividend = divisor.
  CHECK(__udivmoddi4(42, 42, &rem) == 1);
  CHECK(rem == 0);

  // Large dividend, small divisor. Forces shift-and-subtract loop.
  CHECK(__udivmoddi4(0x100000000ULL, 3, &rem) == 0x55555555);
  CHECK(rem == 1);

  // Max value/2.
  CHECK(__udivmoddi4(UINT64_MAX, 2, &rem) == 0x7FFFFFFFFFFFFFFFULL);
  CHECK(rem == 1);

  // Large divisor near max.
  CHECK(__udivmoddi4(UINT64_MAX, UINT64_MAX - 1, &rem) == 1);
  CHECK(rem == 1);

  // Division by zero returns 0, remainder 0.
  CHECK(__udivmoddi4(42, 0, &rem) == 0);
  CHECK(rem == 0);

  // Null remainder pointer.
  CHECK(__udivmoddi4(100, 7, nullptr) == 14);
}
