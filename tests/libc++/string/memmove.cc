#include <doctest/doctest.h>
#include <stdint.h>
#include <string.h>

TEST_CASE("memmove")
{
  const uint8_t val = 42;
  const uint8_t buf[3] = {val, val, val};
  uint8_t buf2[3] = {0};
  auto *res = memmove(buf2, buf, 3);
  CHECK(res == buf2);
  CHECK(memcmp(buf, buf2, 3) == 0);

  // Copy within same buffer (overlapping).
  uint8_t buf3[3] = {1, 2, 3};
  res = memmove(buf3, buf3 + 1, 2);
  CHECK(res == buf3);
  const uint8_t expected[3] = {2, 3, 3};
  CHECK(memcmp(buf3, expected, 3) == 0);
}
