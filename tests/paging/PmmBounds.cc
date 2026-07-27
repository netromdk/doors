#include <kernel/Pmm.h>

#include "PmmTestHooks.h"
#include <doctest/doctest.h>

constexpr size_t TEST_FRAME_COUNT = 256;
constexpr uint32_t TEST_MAX_PHYS = TEST_FRAME_COUNT * Pmm::PAGE_SIZE;

TEST_CASE("maxPhysAddr returns 0 when maxFrameIdx is 0")
{
  pmmTestSetMaxFrameIdx(0);
  CHECK(Pmm::maxPhysAddr() == 0);
}

TEST_CASE("maxPhysAddr returns maxFrameIdx * PAGE_SIZE")
{
  pmmTestSetMaxFrameIdx(1);
  CHECK(Pmm::maxPhysAddr() == Pmm::PAGE_SIZE);

  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::maxPhysAddr() == TEST_MAX_PHYS);

  pmmTestSetMaxFrameIdx(1024);
  CHECK(Pmm::maxPhysAddr() == 1024 * Pmm::PAGE_SIZE);
}

TEST_CASE("isBadPhysAddr rejects 0")
{
  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::isBadPhysAddr(0) == true);
}

TEST_CASE("isBadPhysAddr rejects address at maxPhysAddr")
{
  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::isBadPhysAddr(TEST_MAX_PHYS) == true);
}

TEST_CASE("isBadPhysAddr rejects address above maxPhysAddr")
{
  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::isBadPhysAddr(TEST_MAX_PHYS + 1) == true);
}

TEST_CASE("isBadPhysAddr accepts address below maxPhysAddr")
{
  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::isBadPhysAddr(Pmm::PAGE_SIZE) == false);
}

TEST_CASE("isBadPhysAddr accepts address just below maxPhysAddr")
{
  pmmTestSetMaxFrameIdx(TEST_FRAME_COUNT);
  CHECK(Pmm::isBadPhysAddr(TEST_MAX_PHYS - Pmm::PAGE_SIZE) == false);
}
