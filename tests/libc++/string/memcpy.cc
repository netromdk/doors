#include <doctest/doctest.h>
#include <stdint.h>
#include <string.h>

TEST_CASE("memcpy")
{
  const uint8_t buf[3] = {1, 2, 3};
  uint8_t dst[3] = {0};
  auto *res = memcpy(dst, buf, 3);
  CHECK(res == dst);
  CHECK(dst[0] == 1);
  CHECK(dst[1] == 2);
  CHECK(dst[2] == 3);
}
