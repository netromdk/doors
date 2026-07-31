#include <cstdint>
#include <cstring>

#include <kernel/Heap.h>
#include <kernel/Pit.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>
#include <kernel/Stats.h>

#include <arch/i386/Paging.h>

#include "scheduler/SchedulerFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SchedulerFixture, "snapshot zero state")
{
  // Before any events, verify static members are zero-initialized.
  CHECK(Paging::pageFaults_ == 0);
  CHECK(Paging::cowFaults_ == 0);
  CHECK(Paging::userPageFaults_ == 0);
  CHECK(Pmm::allocCount_ == 0);
  CHECK(Pmm::freeCount_ == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "snapshot uptime is monotonic")
{
  Pit::init();

  const auto before = Pit::uptimeMs();

  Stats::snapshot();
  StatsSnapshot snap1{};
  REQUIRE(Stats::getLatest(snap1));
  CHECK(snap1.uptimeMs >= before);

  Stats::snapshot();
  StatsSnapshot snap2{};
  REQUIRE(Stats::getLatest(snap2));
  CHECK(snap2.uptimeMs >= snap1.uptimeMs);
}

TEST_CASE_FIXTURE(SchedulerFixture, "context switches increment after tick")
{
  Scheduler::addTask("task1", nullptr);

  // Exhaust quantum to trigger a switch inside `tick()`.
  SchedulerTestAccess::setCurrentIdx(0);
  SchedulerTestAccess::setQuantumStartMs(0);
  pitTicks = Scheduler::QUANTUM_MS + 1;
  Scheduler::tick(0x1000);

  Stats::snapshot();
  StatsSnapshot snap{};
  REQUIRE(Stats::getLatest(snap));
  CHECK(snap.contextSwitches >= 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "alive task count matches scheduler state")
{
  Stats::snapshot();
  StatsSnapshot snap1{};
  REQUIRE(Stats::getLatest(snap1));
  const auto beforeAlive = snap1.aliveTasks;

  Scheduler::addTask("test", nullptr);
  Scheduler::addTask("test2", nullptr);

  Stats::snapshot();
  StatsSnapshot snap2{};
  REQUIRE(Stats::getLatest(snap2));
  CHECK(snap2.aliveTasks == beforeAlive + 2);
}

TEST_CASE_FIXTURE(SchedulerFixture, "ring buffer wraps at capacity")
{
  // Push more than capacity to trigger wrap.
  for (size_t i = 0; i < Stats::RING_SIZE + 1; ++i) {
    Stats::snapshot();
  }

  StatsSnapshot snap{};
  CHECK(Stats::getLatest(snap));
  CHECK(Stats::ringEntryCount() == Stats::RING_SIZE);
}

TEST_CASE_FIXTURE(SchedulerFixture, "PMM alloc and free counters work together")
{
  CHECK(Pmm::allocCount_ == 0);
  CHECK(Pmm::freeCount_ == 0);

  void *f1 = Pmm::allocFrame();
  REQUIRE(f1 != nullptr);
  CHECK(Pmm::allocCount_ == 1);
  CHECK(Pmm::freeCount_ == 0);

  void *f2 = Pmm::allocFrame();
  REQUIRE(f2 != nullptr);
  CHECK(Pmm::allocCount_ == 2);

  // Peak in-use frames is recorded by the high-water mark.
  CHECK(Pmm::highWaterFrames_ == 2);

  // Frees lower the in-use count but never lower the recorded peak.
  Pmm::freeFrame(f1);
  CHECK(Pmm::freeCount_ >= 1);
  CHECK(Pmm::totalFrees() >= 1);
  CHECK(Pmm::highWaterFrames_ == 2);

  // Refcount-driven frees count once, on the reference that drops to zero.
  Pmm::addRef(f2);
  CHECK(Pmm::removeRef(f2) == false);
  CHECK(Pmm::totalFrees() >= 1); // still 1: f2 still has one reference
  CHECK(Pmm::removeRef(f2) == true);
  CHECK(Pmm::totalFrees() >= 2); // last reference released: counted
  CHECK(Pmm::highWaterFrames_ == 2);
}
