#include <doctest/doctest.h>
#include <cstdint>
#include <cstring>

TEST_CASE("memset")
{
  uint8_t val = 42;
  uint8_t buf[3] = {1, 2, 3};
  memset(buf, val, 3);
  CHECK(buf[0] == val);
  CHECK(buf[1] == val);
  CHECK(buf[2] == val);
}
