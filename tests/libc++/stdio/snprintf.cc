#include <cstdint>
#include <cstdio>
#include <cstring>

#include <doctest/doctest.h>

struct SnprintfFixture {
  char buf[128]{};
};

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf basic")
{
  CHECK(snprintf(buf, sizeof(buf), "hello") == 5);
  CHECK(strcmp(buf, "hello") == 0);

  CHECK(snprintf(buf, sizeof(buf), "hello\n") == 6);
  CHECK(strcmp(buf, "hello\n") == 0);

  CHECK(snprintf(buf, sizeof(buf), "") == 0);
  CHECK(strcmp(buf, "") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf string arg")
{
  CHECK(snprintf(buf, sizeof(buf), "%s", "world") == 5);
  CHECK(strcmp(buf, "world") == 0);

  CHECK(snprintf(buf, sizeof(buf), "hello %s", "world") == 11);
  CHECK(strcmp(buf, "hello world") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%s\n", "hello") == 6);
  CHECK(strcmp(buf, "hello\n") == 0);

  // `nullptr` string prints (NULL).
  const char *null_str{nullptr};
  CHECK(snprintf(buf, sizeof(buf), "%s", null_str) == 6);
  CHECK(strcmp(buf, "(NULL)") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf int")
{
  CHECK(snprintf(buf, sizeof(buf), "%d", 42) == 2);
  CHECK(strcmp(buf, "42") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", -42) == 3);
  CHECK(strcmp(buf, "-42") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", 0) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned")
{
  CHECK(snprintf(buf, sizeof(buf), "%u", 42u) == 2);
  CHECK(strcmp(buf, "42") == 0);

  const int32_t neg{-1};
  CHECK(snprintf(buf, sizeof(buf), "%u", static_cast<uint32_t>(neg)) == 10);
  CHECK(strcmp(buf, "4294967295") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf hex")
{
  CHECK(snprintf(buf, sizeof(buf), "%x", 255) == 2);
  CHECK(strcmp(buf, "ff") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%X", 255) == 2);
  CHECK(strcmp(buf, "FF") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%x", 3735928559u) == 8);
  CHECK(strcmp(buf, "deadbeef") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%X", 3735928559u) == 8);
  CHECK(strcmp(buf, "DEADBEEF") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf char")
{
  CHECK(snprintf(buf, sizeof(buf), "%c", 'A') == 1);
  CHECK(strcmp(buf, "A") == 0);

  CHECK(snprintf(buf, sizeof(buf), "<%c>", 'Z') == 3);
  CHECK(strcmp(buf, "<Z>") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf bool")
{
  CHECK(snprintf(buf, sizeof(buf), "%d", true) == 1);
  CHECK(strcmp(buf, "1") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d", false) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf bool %b")
{
  CHECK(snprintf(buf, sizeof(buf), "%b", true) == 4);
  CHECK(strcmp(buf, "true") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%b", false) == 5);
  CHECK(strcmp(buf, "false") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf pointer")
{
  const int x{0};
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

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf width")
{
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

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf width with strings")
{
  CHECK(snprintf(buf, sizeof(buf), "%10s", "hi") == 10);
  CHECK(strcmp(buf, "        hi") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%-10s", "hi") == 10);
  CHECK(strcmp(buf, "hi        ") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf literal percent")
{
  CHECK(snprintf(buf, sizeof(buf), "100%%") == 4);
  CHECK(strcmp(buf, "100%") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%d%%", 50) == 3);
  CHECK(strcmp(buf, "50%") == 0);
}

TEST_CASE("snprintf buffer truncation")
{
  char buf[4];

  const int n{snprintf(buf, sizeof(buf), "hello")};
  CHECK(n == 5);
  CHECK(strcmp(buf, "hel") == 0);

  // Null-terminated even when truncated.
  CHECK(buf[3] == '\0');
}

TEST_CASE("snprintf null buffer")
{
  // When s is `nullptr` and n is 0, return the length that would be written.
  const int n{snprintf(nullptr, 0, "hello %s", "world")};
  CHECK(n == 11);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf mixed format")
{
  CHECK(snprintf(buf, sizeof(buf), "%s %d 0x%x", "val", 255, 255) == 12);
  CHECK(strcmp(buf, "val 255 0xff") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf int64")
{
  // Verify it doesn't crash and produces something.
  const int64_t big{1234567890123LL};
  CHECK(snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(big)) > 0);
  CHECK(strlen(buf) > 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf uint64")
{
  const uint64_t big{18446744073709551615ULL};
  CHECK(snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(big)) > 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "zero-pad with left-align: %-08d same as %-8d")
{
  CHECK(snprintf(buf, sizeof(buf), "%-08d", 42) == 8);
  CHECK(strcmp(buf, "42      ") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf short (SignedInteger concept)")
{
  const short v{-12345};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 6);
  CHECK(strcmp(buf, "-12345") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned short (UnsignedInteger concept)")
{
  const unsigned short v{65000};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 5);
  CHECK(strcmp(buf, "65000") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf long (SignedInteger concept)")
{
  const long v{-1234567890L};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 11);
  CHECK(strcmp(buf, "-1234567890") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned long (UnsignedInteger concept)")
{
  const unsigned long v{3735928559UL};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 10);
  CHECK(strcmp(buf, "3735928559") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf long long (SignedInteger concept, 64-bit)")
{
  const long long v{1234567890123LL};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) > 0);
  CHECK(strlen(buf) > 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf size_t (UnsignedInteger concept)")
{
  const size_t v{1024};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 4);
  CHECK(strcmp(buf, "1024") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture,
                  "snprintf int64_t via %u (SignedInteger concept unsigned dispatch 64-bit)")
{
  // %u triggers unsigned dispatch in the signed branch, casting -1 to `uint64_t`.
  const int64_t v{-1};
  CHECK(snprintf(buf, sizeof(buf), "%u", static_cast<long long>(v)) == 20);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned long long (UnsignedInteger concept, 64-bit)")
{
  const unsigned long long v{18446744073709551615ULL};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) > 0);
  CHECK(strlen(buf) > 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf signed int8_t (SignedInteger concept)")
{
  const int8_t v{-128};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 4);
  CHECK(strcmp(buf, "-128") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture,
                  "snprintf unsigned via signed with %u (SignedInteger concept unsigned dispatch)")
{
  const int v{-1};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 10);
  CHECK(strcmp(buf, "4294967295") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned char as char (%c path)")
{
  const unsigned char v{'A'};
  CHECK(snprintf(buf, sizeof(buf), "%c", v) == 1);
  CHECK(strcmp(buf, "A") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf unsigned char as unsigned integer (%u path)")
{
  const unsigned char v{200};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 3);
  CHECK(strcmp(buf, "200") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf char as unsigned (%u dispatch)")
{
  const char v{-1};
  CHECK(snprintf(buf, sizeof(buf), "%u", v) == 3);
  CHECK(strcmp(buf, "255") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf char as signed integer (%d path)")
{
  const char v{-100};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 4);
  CHECK(strcmp(buf, "-100") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf char positive as signed integer (%d path)")
{
  const char v{'A'};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 2);
  CHECK(strcmp(buf, "65") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf octal %o")
{
  CHECK(snprintf(buf, sizeof(buf), "%o", 255) == 3);
  CHECK(strcmp(buf, "377") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%o", 8u) == 2);
  CHECK(strcmp(buf, "10") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%o", 0) == 1);
  CHECK(strcmp(buf, "0") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf binary %b with integer (not bool)")
{
  CHECK(snprintf(buf, sizeof(buf), "%b", 6) == 3);
  CHECK(strcmp(buf, "110") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%b", 0u) == 1);
  CHECK(strcmp(buf, "0") == 0);

  CHECK(snprintf(buf, sizeof(buf), "%b", 255) == 8);
  CHECK(strcmp(buf, "11111111") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf uppercase hex %X with signed int")
{
  CHECK(snprintf(buf, sizeof(buf), "%X", 255) == 2);
  CHECK(strcmp(buf, "FF") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf long long negative (64-bit signed path)")
{
  const long long v{-1234567890123LL};
  CHECK(snprintf(buf, sizeof(buf), "%d", v) == 14);
  CHECK(strcmp(buf, "-1234567890123") == 0);
}

TEST_CASE_FIXTURE(SnprintfFixture, "snprintf LLONG_MIN (64-bit signed edge case)")
{
  const long long v{-9223372036854775807LL - 1};
  const int n{snprintf(buf, sizeof(buf), "%d", v)};
  CHECK(n == 20);
  CHECK(strcmp(buf, "-9223372036854775808") == 0);
}
