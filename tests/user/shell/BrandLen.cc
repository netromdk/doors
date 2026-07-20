#include "Util.h"

#include <doctest/doctest.h>

TEST_CASE("brandLen: empty string")
{
  CHECK(brandLen("") == 0);
}

TEST_CASE("brandLen: short string")
{
  CHECK(brandLen("hello") == 5);
}

TEST_CASE("brandLen: exact 48 chars")
{
  char buf[49];
  for (int i = 0; i < 48; ++i) {
    buf[i] = static_cast<char>('a' + (i % 26));
  }
  buf[48] = '\0';
  CHECK(brandLen(buf) == 48);
}

TEST_CASE("brandLen: truncated at 48")
{
  char buf[64];
  for (int i = 0; i < 60; ++i) {
    buf[i] = 'x';
  }
  buf[60] = '\0';
  CHECK(brandLen(buf) == 48);
}
