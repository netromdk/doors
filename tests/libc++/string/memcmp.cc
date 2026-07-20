#include <doctest/doctest.h>
#include <cstdint>
#include <cstring>

TEST_CASE("memcmp")
{
  const uint8_t buf[3] = {1, 2, 3};
  const uint8_t buf2[3] = {1, 2, 3};
  CHECK(memcmp(buf, buf2, 3) == 0);

  const uint8_t buf3[3] = {3, 2, 1};
  CHECK(memcmp(buf, buf3, 3) == -1);

  // 3 < 1
  CHECK(memcmp(buf3, buf, 3) == 1);
}
