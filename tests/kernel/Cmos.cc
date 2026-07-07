#include <doctest/doctest.h>
#include <kernel/Cmos.h>

TEST_CASE("Cmos::pm12To24: 12 PM")
{
  CHECK(Cmos::pm12To24(0x8C) == 12);
}

TEST_CASE("Cmos::pm12To24: 1 PM")
{
  CHECK(Cmos::pm12To24(0x81) == 13);
}

TEST_CASE("Cmos::pm12To24: 3 PM")
{
  CHECK(Cmos::pm12To24(0x83) == 15);
}

TEST_CASE("Cmos::pm12To24: 11 PM")
{
  CHECK(Cmos::pm12To24(0x8B) == 23);
}

TEST_CASE("Cmos::pm12To24: 12 AM")
{
  CHECK(Cmos::pm12To24(0x0C) == 12);
}

TEST_CASE("Cmos::pm12To24: 1 AM")
{
  CHECK(Cmos::pm12To24(0x01) == 13);
}

TEST_CASE("Cmos::pm12To24: 11 AM")
{
  CHECK(Cmos::pm12To24(0x0B) == 23);
}
