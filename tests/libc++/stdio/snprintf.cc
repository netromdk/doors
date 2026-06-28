#include <cstring>
#include <doctest/doctest.h>
#include <stdio.h>

TEST_CASE("snprintf basic")
{
  char buf[128];

  // Simple string.
  CHECK(snprintf(buf, sizeof(buf), "hello") == 5);
  CHECK(strcmp(buf, "hello") == 0);

  // With newline.
  CHECK(snprintf(buf, sizeof(buf), "hello\n") == 6);
  CHECK(strcmp(buf, "hello\n") == 0);

  // Empty string.
  CHECK(snprintf(buf, sizeof(buf), "") == 0);
  CHECK(strcmp(buf, "") == 0);
}

TEST_CASE("snprintf string arg")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%s", "world") == 5);
  CHECK(strcmp(buf, "world") == 0);

  CHECK(snprintf(buf, sizeof(buf), "hello %s", "world") == 11);
  CHECK(strcmp(buf, "hello world") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%s\n", "hello") == 6);
  CHECK(strcmp(buf, "hello\n") == 0);

  // `nullptr` string prints (NULL).
  const char *null_str = nullptr;
  CHECK(snprintf(buf, sizeof(buf), "%s", null_str) == 6);
  CHECK(strcmp(buf, "(NULL)") == 0);
}

TEST_CASE("snprintf int")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%d", 42) == 2);
  CHECK(strcmp(buf, "42") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", -42) == 3);
  CHECK(strcmp(buf, "-42") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", 0) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE("snprintf unsigned")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%u", 42u) == 2);
  CHECK(strcmp(buf, "42") == 0);

  int32_t neg = -1;
  CHECK(snprintf(buf, sizeof(buf), "%u", (uint32_t) neg) == 10);
  CHECK(strcmp(buf, "4294967295") == 0);
}

TEST_CASE("snprintf hex")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%x", 255) == 2);
  CHECK(strcmp(buf, "ff") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%X", 255) == 2);
  CHECK(strcmp(buf, "FF") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%x", 3735928559u) == 8);
  CHECK(strcmp(buf, "deadbeef") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%X", 3735928559u) == 8);
  CHECK(strcmp(buf, "DEADBEEF") == 0);
}

TEST_CASE("snprintf char")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%c", 'A') == 1);
  CHECK(strcmp(buf, "A") == 0);

  CHECK(snprintf(buf, sizeof(buf), "<%c>", 'Z') == 3);
  CHECK(strcmp(buf, "<Z>") == 0);
}

TEST_CASE("snprintf bool")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%d", true) == 1);
  CHECK(strcmp(buf, "1") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", false) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE("snprintf bool %b")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%b", true) == 4);
  CHECK(strcmp(buf, "true") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%b", false) == 5);
  CHECK(strcmp(buf, "false") == 0);
}

TEST_CASE("snprintf pointer")
{
  char buf[128];

  int x = 0;
  int n = snprintf(buf, sizeof(buf), "%p", &x);
  CHECK(n > 2);
  CHECK(buf[0] == '0');
  CHECK(buf[1] == 'x');

  // Null pointer.
  n = snprintf(buf, sizeof(buf), "%p", nullptr);
  CHECK(n > 2);
  CHECK(buf[0] == '0');
  CHECK(buf[1] == 'x');
}

TEST_CASE("snprintf width")
{
  char buf[128];

  // Right-aligned with spaces (default).
  CHECK(snprintf(buf, sizeof(buf), "%8d", 42) == 8);
  CHECK(strcmp(buf, "      42") == 0);

  // Left-aligned.
  CHECK(snprintf(buf, sizeof(buf), "%-8d", 42) == 8);
  CHECK(strcmp(buf, "42      ") == 0);

  // Zero-padded.
  CHECK(snprintf(buf, sizeof(buf), "%08d", 42) == 8);
  CHECK(strcmp(buf, "00000042") == 0);
}

TEST_CASE("snprintf width with strings")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%10s", "hi") == 10);
  CHECK(strcmp(buf, "        hi") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%-10s", "hi") == 10);
  CHECK(strcmp(buf, "hi        ") == 0);
}

TEST_CASE("snprintf literal percent")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "100%%") == 4);
  CHECK(strcmp(buf, "100%") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d%%", 50) == 3);
  CHECK(strcmp(buf, "50%") == 0);
}

TEST_CASE("snprintf buffer truncation")
{
  char buf[4];

  int n = snprintf(buf, sizeof(buf), "hello");
  CHECK(n == 5);                  // would have written 5
  CHECK(strcmp(buf, "hel") == 0); // but only 3 chars + null fit

  // Null-terminated even when truncated.
  CHECK(buf[3] == '\0');
}

TEST_CASE("snprintf null buffer")
{
  // When s is `nullptr` and n is 0, return the length that would be written.
  int n = snprintf(nullptr, 0, "hello %s", "world");
  CHECK(n == 11);
}

TEST_CASE("snprintf mixed format")
{
  char buf[128];

  CHECK(snprintf(buf, sizeof(buf), "%s %d 0x%x", "val", 255, 255) == 12);
  CHECK(strcmp(buf, "val 255 0xff") == 0);
}

TEST_CASE("snprintf int64")
{
  char buf[128];
  int64_t big = 1234567890123LL;

  CHECK(snprintf(buf, sizeof(buf), "%lld", (long long) big) > 0);

  // Verify it doesn't crash and produces something.
  CHECK(strlen(buf) > 0);
}

TEST_CASE("snprintf uint64")
{
  char buf[128];
  uint64_t big = 18446744073709551615ULL;

  CHECK(snprintf(buf, sizeof(buf), "%llu", (unsigned long long) big) > 0);
}

TEST_CASE("sprintf basic")
{
  char buf[128];

  CHECK(sprintf(buf, "hello %s", "world") == 11);
  CHECK(strcmp(buf, "hello world") == 0);

  CHECK(sprintf(buf, "%d", 42) == 2);
  CHECK(strcmp(buf, "42") == 0);
}

TEST_CASE("zero-pad with left-align: %-08d same as %-8d")
{
  char buf[128];
  CHECK(snprintf(buf, sizeof(buf), "%-08d", 42) == 8);
  CHECK(strcmp(buf, "42      ") == 0);
}
