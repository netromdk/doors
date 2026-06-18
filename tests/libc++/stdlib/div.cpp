#include <doctest/doctest.h>
#include <stdlib.h>

TEST_CASE("div") {
  div_t<int> res = div(38, 5);
  CHECK(res.quot == 7);
  CHECK(res.rem == 3);

  res = div(31558149, 3600);
  CHECK(res.quot == 8766);
  CHECK(res.rem == 549);
}
