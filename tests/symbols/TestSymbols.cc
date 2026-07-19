#include <doctest/doctest.h>
#include <kernel/Symbols.h>

TEST_CASE("lookupSymbol exact match")
{
  const char *s = lookupSymbol(0x100000); // {0x100000, "_start"}
  CHECK(s != nullptr);
  CHECK(s[0] == '_');
}

TEST_CASE("lookupSymbol exact match kmain")
{
  const char *s = lookupSymbol(0x100020); // {0x100020, "kmain"}
  CHECK(s != nullptr);
}

TEST_CASE("lookupSymbol match inside function")
{
  // 0x100020 <= 0x100022 < 0x100100 -> "kmain"
  const char *s = lookupSymbol(0x100022);
  CHECK(s != nullptr);
}

TEST_CASE("lookupSymbol before first symbol")
{
  // 0x0FFFFF < 0x100000 -> nullptr
  const char *s = lookupSymbol(0x0FFFFF);
  CHECK(s == nullptr);
}

TEST_CASE("lookupSymbol after last symbol")
{
  // 0xFFFFFF >= 0x102100 -> sentinel(nullptr)
  const char *s = lookupSymbol(0xFFFFFF);
  CHECK(s == nullptr);
}

TEST_CASE("lookupSymbol between symbols")
{
  // 0x100020 <= 0x100090 < 0x100100 -> "kmain"
  const char *s = lookupSymbol(0x100090);
  CHECK(s != nullptr);
}

TEST_CASE("lookupSymbol last symbol exact")
{
  const char *s = lookupSymbol(0x102000); // {0x102000, "dumpBacktrace"}
  CHECK(s != nullptr);
}
