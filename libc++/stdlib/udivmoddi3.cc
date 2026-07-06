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

  // Subtract where the divisor fits.
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

// Combined 64-bit unsigned division + modulo.
// Returns quotient and stores remainder in `rem`.
uint64_t __udivmoddi4(uint64_t a, uint64_t b, uint64_t *rem)
{
  if (rem) {
    *rem = 0;
  }

  if (b == 0) {
    return 0;
  }

  const auto a_lo = static_cast<uint32_t>(a);
  const auto a_hi = static_cast<uint32_t>(a >> 32);
  const auto b_lo = static_cast<uint32_t>(b);
  if (a_hi == 0 && b_lo <= a_lo) {
    if (rem) {
      *rem = a_lo % b_lo;
    }
    return a_lo / b_lo;
  }

  uint64_t quotient = 0;
  int shift = 0;
  uint64_t divisor = b;
  while (divisor <= a && !(divisor & (1ULL << 63))) {
    divisor <<= 1;
    ++shift;
  }

  while (shift >= 0) {
    if (a >= divisor) {
      a -= divisor;
      quotient |= (1ULL << shift);
    }
    divisor >>= 1;
    --shift;
  }

  if (rem) {
    *rem = a;
  }
  return quotient;
}
}
