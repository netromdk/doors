#include <cstdint>

#include <kernel/Pit.h>

#include <doctest/doctest.h>

extern volatile uint64_t pitTicks;

TEST_CASE("tickless: init sets deadline to 1")
{
  pitTicks = 0;
  Pit::init();
  CHECK(Pit::deadline() == 1);
}

TEST_CASE("tickless: programForMs sets deadline")
{
  pitTicks = 100;
  Pit::programForMs(50);
  CHECK(Pit::deadline() == 150);
}

TEST_CASE("tickless: tick updates pitTicks to deadline")
{
  pitTicks = 0;
  Pit::programForMs(50);
  Pit::tick();
  CHECK(Pit::uptimeMs() == 50);
}

TEST_CASE("tickless: programForMs clamps to max")
{
  pitTicks = 0;
  Pit::programForMs(PIT_MAX_MS * 10);
  CHECK(Pit::deadline() == PIT_MAX_MS);
}

TEST_CASE("tickless: programForMs clamps zero to 1")
{
  pitTicks = 0;
  Pit::programForMs(0);
  CHECK(Pit::deadline() == 1);
}

TEST_CASE("tickless: successive programForMs updates deadline")
{
  pitTicks = 0;
  Pit::programForMs(50);
  CHECK(Pit::deadline() == 50);
  Pit::tick();
  CHECK(Pit::uptimeMs() == 50);

  Pit::programForMs(30);
  CHECK(Pit::deadline() == 80);
  Pit::tick();
  CHECK(Pit::uptimeMs() == 80);
}
