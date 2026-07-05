#include <doctest/doctest.h>
#include <stdint.h>

extern "C" uint64_t __udivdi3(uint64_t a, uint64_t b);
extern "C" uint64_t __umoddi3(uint64_t a, uint64_t b);

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
