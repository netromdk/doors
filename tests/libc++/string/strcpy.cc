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

  // Check that null terminator is copied.
  char buf3[4];
  memset(buf3, 0xFF, sizeof(buf3));
  strcpy(buf3, "hi");
  CHECK(buf3[0] == 'h');
  CHECK(buf3[1] == 'i');
  CHECK(buf3[2] == '\0');
  CHECK(buf3[3] == '\xFF');

  // Empty string copies just a null terminator.
  char buf4[2];
  memset(buf4, 0xFF, sizeof(buf4));
  strcpy(buf4, "");
  CHECK(buf4[0] == '\0');
  CHECK(buf4[1] == '\xFF');
}
