#include <algorithm>
#include <doctest/doctest.h>

TEST_CASE("clamp in range")
{
  CHECK(clamp(5, 1, 10) == 5);
}

TEST_CASE("clamp below low")
{
  CHECK(clamp(0, 1, 10) == 1);
}

TEST_CASE("clamp above high")
{
  CHECK(clamp(15, 1, 10) == 10);
}

TEST_CASE("clamp returns reference to lo when below")
{
  const int lo = 1;
  const int hi = 10;
  CHECK(&clamp(lo - 1, lo, hi) == &lo);
}

TEST_CASE("clamp returns reference to hi when above")
{
  const int lo = 1;
  const int hi = 10;
  CHECK(&clamp(hi + 1, lo, hi) == &hi);
}

TEST_CASE("clamp returns reference to value when in range")
{
  const int v = 5, a = 1, b = 10;
  CHECK(&clamp(v, a, b) == &v);
}

TEST_CASE("clamp returns correct value when in range")
{
  const int v = 5, a = 1, b = 10;
  CHECK(clamp(v, a, b) == 5);
}

TEST_CASE("clamp returns reference to value at low boundary")
{
  const int a = 1, b = 10;
  CHECK(&clamp(a, a, b) == &a);
}

TEST_CASE("clamp returns reference to value at high boundary")
{
  const int a = 1, b = 10;
  CHECK(&clamp(b, a, b) == &b);
}

TEST_CASE("clamp returns reference to value when lo equals v")
{
  const int v = 5, b = 10;
  CHECK(&clamp(v, v, b) == &v);
}

TEST_CASE("clamp returns reference to value when hi equals v")
{
  const int v = 5, a = 1;
  CHECK(&clamp(v, a, v) == &v);
}
