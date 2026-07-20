#include <cstdint>
#include <cstdio>
#include <cstring>

#include <doctest/doctest.h>

struct SprintfFixture {
  char buf[128]{};
};

TEST_CASE_FIXTURE(SprintfFixture, "sprintf basic")
{
  CHECK(sprintf(buf, "hello %s", "world") == 11);
  CHECK(strcmp(buf, "hello world") == 0);
  CHECK(sprintf(buf, "%d", 42) == 2);
  CHECK(strcmp(buf, "42") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf string")
{
  CHECK(sprintf(buf, "%s", "world") == 5);
  CHECK(strcmp(buf, "world") == 0);

  const char *null_str = nullptr;
  CHECK(sprintf(buf, "%s", null_str) == 6);
  CHECK(strcmp(buf, "(NULL)") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf int")
{
  CHECK(sprintf(buf, "%d", 42) == 2);
  CHECK(strcmp(buf, "42") == 0);
  CHECK(sprintf(buf, "%d", -42) == 3);
  CHECK(strcmp(buf, "-42") == 0);
  CHECK(sprintf(buf, "%d", 0) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf unsigned")
{
  CHECK(sprintf(buf, "%u", 42u) == 2);
  CHECK(strcmp(buf, "42") == 0);

  const int32_t neg{-1};
  CHECK(sprintf(buf, "%u", static_cast<uint32_t>(neg)) == 10);
  CHECK(strcmp(buf, "4294967295") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf hex")
{
  CHECK(sprintf(buf, "%x", 255) == 2);
  CHECK(strcmp(buf, "ff") == 0);
  CHECK(sprintf(buf, "%X", 255) == 2);
  CHECK(strcmp(buf, "FF") == 0);
  CHECK(sprintf(buf, "%x", 3735928559u) == 8);
  CHECK(strcmp(buf, "deadbeef") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf char")
{
  CHECK(sprintf(buf, "%c", 'A') == 1);
  CHECK(strcmp(buf, "A") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf pointer")
{
  const int x{0};
  int n{sprintf(buf, "%p", &x)};
  CHECK(n > 2);
  CHECK(buf[0] == '0');
  CHECK(buf[1] == 'x');

  n = sprintf(buf, "%p", nullptr);
  CHECK(n > 2);
  CHECK(buf[0] == '0');
  CHECK(buf[1] == 'x');
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf width")
{
  CHECK(sprintf(buf, "%8d", 42) == 8);
  CHECK(strcmp(buf, "      42") == 0);
  CHECK(sprintf(buf, "%-8d", 42) == 8);
  CHECK(strcmp(buf, "42      ") == 0);
  CHECK(sprintf(buf, "%08d", 42) == 8);
  CHECK(strcmp(buf, "00000042") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf width with strings")
{
  CHECK(sprintf(buf, "%10s", "hi") == 10);
  CHECK(strcmp(buf, "        hi") == 0);
  CHECK(sprintf(buf, "%-10s", "hi") == 10);
  CHECK(strcmp(buf, "hi        ") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf literal percent")
{
  CHECK(sprintf(buf, "100%%") == 4);
  CHECK(strcmp(buf, "100%") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf mixed format")
{
  CHECK(sprintf(buf, "%s %d 0x%x", "val", 255, 255) == 12);
  CHECK(strcmp(buf, "val 255 0xff") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf octal")
{
  CHECK(sprintf(buf, "%o", 255) == 3);
  CHECK(strcmp(buf, "377") == 0);
  CHECK(sprintf(buf, "%o", 0) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf binary")
{
  CHECK(sprintf(buf, "%b", 6) == 3);
  CHECK(strcmp(buf, "110") == 0);
  CHECK(sprintf(buf, "%b", 0u) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf int64")
{
  const int64_t big{1234567890123LL};
  const int n{sprintf(buf, "%lld", static_cast<long long>(big))};
  CHECK(n > 0);
  CHECK(strlen(buf) > 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf LLONG_MIN")
{
  const long long v{-9223372036854775807LL - 1};
  const int n{sprintf(buf, "%d", v)};
  CHECK(n == 20);
  CHECK(strcmp(buf, "-9223372036854775808") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf signed char")
{
  const int8_t v{-128};
  CHECK(sprintf(buf, "%d", v) == 4);
  CHECK(strcmp(buf, "-128") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf unsigned char")
{
  const unsigned char v{200};
  CHECK(sprintf(buf, "%u", v) == 3);
  CHECK(strcmp(buf, "200") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf size_t")
{
  const size_t v{1024};
  CHECK(sprintf(buf, "%u", v) == 4);
  CHECK(strcmp(buf, "1024") == 0);
}

TEST_CASE_FIXTURE(SprintfFixture, "sprintf zero-pad with left-align")
{
  CHECK(sprintf(buf, "%-08d", 42) == 8);
  CHECK(strcmp(buf, "42      ") == 0);
}
