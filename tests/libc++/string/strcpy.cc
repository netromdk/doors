#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strcpy")
{
  const char *msg = "hello world";
  char buf[64] = {0};
  char *res = strcpy(buf, msg);
  CHECK(res == buf);
  CHECK(strlen(buf) == strlen(msg));
  CHECK(strcmp(buf, msg) == 0);

  char buf2[64] = {0};
  char *ptr = buf2 + 10;
  res = strcpy(ptr, msg);
  CHECK(res == ptr);
  CHECK(strlen(ptr) == strlen(msg));
  CHECK(strcmp(ptr, msg) == 0);
}
