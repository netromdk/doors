#include <doctest/doctest.h>
#include <kernel/Pit.h>
#include <cstdint>

extern volatile uint64_t pitTicks;

TEST_CASE("simple_ms_since")
{
  pitTicks = 105;
  CHECK(Pit::msSince(100) == 5);
}

TEST_CASE("wrap_ms_since")
{
  pitTicks = 10;
  CHECK(Pit::msSince(UINT64_MAX - 5) == 16);
}

TEST_CASE("zero")
{
  pitTicks = 0;
  CHECK(Pit::uptimeSec() == 0);
}

TEST_CASE("truncation")
{
  pitTicks = 999;
  CHECK(Pit::uptimeSec() == 0);
}

TEST_CASE("exact_second")
{
  pitTicks = 1000;
  CHECK(Pit::uptimeSec() == 1);
}

TEST_CASE("multi_second")
{
  pitTicks = 5500;
  CHECK(Pit::uptimeSec() == 5);
}

TEST_CASE("order")
{
  pitTicks = 40;
  CHECK(Pit::msSince(50) == UINT64_MAX - 9);
}
