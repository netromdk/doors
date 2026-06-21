#include <doctest/doctest.h>
#include <stdint.h>
#include <string.h>

TEST_CASE("memchr")
{
  const uint8_t buf[3] = {1, 2, 3};
  CHECK(memchr(buf, 2, 3) == buf + 1);
  CHECK(memchr(buf, 3, 3) == buf + 2);

  // Search for something nonexistent.
  CHECK(memchr(buf, 42, 3) == nullptr);
}
