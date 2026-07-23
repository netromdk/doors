#include <cstdint>

#include <kernel/Pmm.h>

#include "PmmTestHooks.h"
#include <doctest/doctest.h>

TEST_CASE("allocFrame sets refcount to 1")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  CHECK(pmmTestRefcount(frame) == 1);
  Pmm::freeFrame(frame);
}

TEST_CASE("freeFrame with refcount 1 frees the frame")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);
  REQUIRE(pmmTestRefcount(frame) == 1);

  pmmTestResetCounts();
  Pmm::freeFrame(frame);
  CHECK(pmmTestFreeCount() == 1);
}

TEST_CASE("freeFrame with refcount > 1 decrements but does not free")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  Pmm::addRef(frame);
  REQUIRE(pmmTestRefcount(frame) == 2);

  pmmTestResetCounts();
  Pmm::freeFrame(frame);
  CHECK(pmmTestRefcount(frame) == 1);

  // Frame is still allocated because refcount > 0.
  CHECK(pmmTestRefcount(frame) == 1);
}

TEST_CASE("addRef increments refcount")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);
  CHECK(pmmTestRefcount(frame) == 1);

  Pmm::addRef(frame);
  CHECK(pmmTestRefcount(frame) == 2);

  Pmm::addRef(frame);
  CHECK(pmmTestRefcount(frame) == 3);

  Pmm::freeFrame(frame);
  Pmm::freeFrame(frame);
  Pmm::freeFrame(frame);
}

TEST_CASE("removeRef decrements refcount, returns true when zero")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  Pmm::addRef(frame); // refcount = 2

  CHECK(Pmm::removeRef(frame) == false); // refcount = 1
  CHECK(pmmTestRefcount(frame) == 1);

  CHECK(Pmm::removeRef(frame) == true); // refcount = 0
  CHECK(pmmTestRefcount(frame) == 0);

  Pmm::freeFrame(frame);
}

TEST_CASE("removeRef returns false when refcount > 0")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  Pmm::addRef(frame); // refcount = 2
  Pmm::addRef(frame); // refcount = 3

  CHECK(Pmm::removeRef(frame) == false); // refcount = 2
  CHECK(Pmm::removeRef(frame) == false); // refcount = 1

  Pmm::freeFrame(frame); // refcount 1 -> 0, freed
}

TEST_CASE("double free panics (print + no-op in stub)")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  Pmm::freeFrame(frame); // refcount 1 -> 0, freed
  pmmTestResetCounts();
  Pmm::freeFrame(frame);          // second free: refcount already 0, should print and not crash.
  CHECK(pmmTestFreeCount() == 1); // `freeFrame()` was called (but did nothing).
}

TEST_CASE("refCount returns correct value")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);
  CHECK(pmmTestRefcount(frame) == 1);

  Pmm::addRef(frame);
  CHECK(pmmTestRefcount(frame) == 2);

  Pmm::addRef(frame);
  CHECK(pmmTestRefcount(frame) == 3);

  Pmm::freeFrame(frame); // refcount = 2
  CHECK(pmmTestRefcount(frame) == 2);

  Pmm::freeFrame(frame); // refcount = 1
  CHECK(pmmTestRefcount(frame) == 1);

  Pmm::freeFrame(frame); // refcount = 0, frame freed
  CHECK(pmmTestRefcount(frame) == 0);
}

TEST_CASE("reserveFrame sets refcount to 1")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  // Put the frame back on the free list so `reserveFrame()` can pull it.
  Pmm::freeFrame(frame);
  CHECK(pmmTestRefcount(frame) == 0);

  // `reserveFrame()` removes the frame from the free list and sets refcount to 1, just like
  // `allocFrame()` does.
  Pmm::reserveFrame(frame);
  CHECK(pmmTestRefcount(frame) == 1);

  Pmm::freeFrame(frame);
}

TEST_CASE("saturation at MAX_REFCOUNT")
{
  auto *frame = Pmm::allocFrame();
  REQUIRE(frame != nullptr);

  // Bump refcount to `MAX_REFCOUNT`.
  for (int i = 1; i < Pmm::MAX_REFCOUNT; ++i) {
    Pmm::addRef(frame);
  }
  CHECK(pmmTestRefcount(frame) == Pmm::MAX_REFCOUNT);

  // One more `addRef()` should saturate, not overflow.
  Pmm::addRef(frame);
  CHECK(pmmTestRefcount(frame) == Pmm::MAX_REFCOUNT);

  // Clean up: remove all refs.
  for (int i = 0; i < Pmm::MAX_REFCOUNT; ++i) {
    Pmm::removeRef(frame);
  }
  CHECK(pmmTestRefcount(frame) == 0);
}
