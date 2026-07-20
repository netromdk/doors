#include <doctest/doctest.h>
#include <cstring>

TEST_CASE("strchr")
{
  const char *msg = "hello world";
  CHECK(strchr(msg, 'l') == msg + 2);
  CHECK(strchr(msg, 'w') == msg + 6);

  // Searching for \0 finds the terminator.
  CHECK(strchr(msg, '\0') == msg + 11);

  // Not found: must return nullptr.
  CHECK(strchr(msg, 'H') == nullptr);
  CHECK(strchr(msg, 'x') == nullptr);
  CHECK(strchr("", 'a') == nullptr);
}
