#include <cstdint>

extern "C" {

// 64-bit unsigned division using shift-and-subtract binary long division.
uint64_t __udivdi3(uint64_t a, uint64_t b)
{
  if (b == 0) {
    return 0;
  }

  // Do 32-bit division natively when possible.
  const auto a_lo = static_cast<uint32_t>(a);
  const auto a_hi = static_cast<uint32_t>(a >> 32);
  const auto b_lo = static_cast<uint32_t>(b);
  if (a_hi == 0 && b_lo <= a_lo) {
    return a_lo / b_lo;
  }

  // Align the divisor so its most significant bit (MSB) matches the divisor's MSB. Guarding against
  // bit 63 prevents shifting the sign bit into the 64th position, which would make `b` appear >=
  // `a`.
  uint64_t quotient = 0;
  int shift = 0;
  while (b <= a && !(b & (1ULL << 63))) {
    b <<= 1;
    ++shift;
  }

  // Ssubtract where the divisor fits.
  while (shift >= 0) {
    if (a >= b) {
      a -= b;
      quotient |= (1ULL << shift);
    }
    b >>= 1;
    --shift;
  }

  return quotient;
}

// 64-bit unsigned modulo. Same as `__udivdi3` but only tracks the remainder.
uint64_t __umoddi3(uint64_t a, uint64_t b)
{
  if (b == 0) {
    return 0;
  }

  // Do 32-bit division natively when possible.
  const auto a_lo = static_cast<uint32_t>(a);
  const auto a_hi = static_cast<uint32_t>(a >> 32);
  const auto b_lo = static_cast<uint32_t>(b);
  if (a_hi == 0 && b_lo <= a_lo) {
    return a_lo % b_lo;
  }

  int shift = 0;
  while (b <= a && !(b & (1ULL << 63))) {
    b <<= 1;
    ++shift;
  }

  while (shift >= 0) {
    if (a >= b) {
      a -= b;
    }
    b >>= 1;
    --shift;
  }

  return a; // Remainder of the division.
}
}
