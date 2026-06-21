#include <doctest/doctest.h>
#include <string.h>

TEST_CASE("strrchr")
{
  const char *msg = "hello world";
  CHECK(strrchr(msg, 'l') == msg + 9);
  CHECK(strrchr(msg, 'o') == msg + 7);

  // Searching for \0 finds the terminator.
  CHECK(strrchr(msg, '\0') == msg + 11);

  // Not found: implementation returns start of string.
  CHECK(strrchr(msg, 'H') == msg);
}
