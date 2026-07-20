#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strncpy")
{
  const char *msg = "hello world";
  char buf[64] = {0};
  size_t num = 6;
  const char *res = strncpy(buf, msg, num);
  CHECK(res == buf);
  CHECK(strlen(buf) == num);
  CHECK(strncmp(buf, msg, num) == 0);

  // Copy shorter string than bytes to copy: remainder gets zero-padded.
  char buf2[64];
  num = 20;
  res = strncpy(buf2, msg, num);
  CHECK(res == buf2);
  CHECK(strlen(buf2) == strlen(msg));
}
