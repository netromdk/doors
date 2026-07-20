#include <cstdint>

#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include <arch/i386/Paging.h>

#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>

extern volatile uint64_t pitTicks;
extern "C" uint32_t syscallFrameEsp;

struct ExecFixture {
  alignas(16) static inline uint8_t pool[262144];

  ExecFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    SchedulerTestAccess::resetTotalExited();
    syscallFrameEsp = 0;
  }

  int addRing0Task(const char *name)
  {
    const auto id = Scheduler::addTask(name, nullptr);
    REQUIRE(id);
    return *id;
  }

  int addRing3Task(const char *name)
  {
    const auto id = Scheduler::addTask(name, nullptr);
    REQUIRE(id);

    auto *t = SchedulerTestAccess::getTask(*id);
    t->userStackPageCount = 2;
    t->userStackVaddr[0] = 0xB0000000;
    t->userStackVaddr[1] = 0xAFFFE000;
    t->userStackPhys[0] = 0x00501000;
    t->userStackPhys[1] = 0x00502000;
    t->pageDir = Paging::clonePageDir();
    t->elfPageCount = 1;
    t->elfVaddr[0] = 0x10000000;
    t->elfPhys[0] = 0x00600000;
    return *id;
  }
};

TEST_CASE_FIXTURE(ExecFixture, "exec: invalid module index returns -1")
{
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);

  // PmmStub returns `moduleCount() == 0`, so any index is invalid.
  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));
  CHECK(Scheduler::exec(-1) == static_cast<uint32_t>(-1));
  CHECK(Scheduler::exec(99) == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ExecFixture, "exec: ring-0 task returns -1")
{
  const auto id = addRing0Task("kernel_task");
  SchedulerTestAccess::setCurrentIdx(id);

  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ExecFixture, "exec: invalid ELF returns -1")
{
  // Add a ring-3 task so exec gets past the ring-0 check.
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);

  // `moduleCount()` returns 0 from stub, so exec fails on index check before even reaching ELF
  // validation.
  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ExecFixture, "exec: preserves PID after failure")
{
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);
  const auto *t = SchedulerTestAccess::getTask(id);
  const auto pidBefore = t->pid;

  Scheduler::exec(0);

  CHECK(t->pid == pidBefore);
}

TEST_CASE_FIXTURE(ExecFixture, "exec: clears elfPageCount on failure path")
{
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);
  auto *t = SchedulerTestAccess::getTask(id);
  REQUIRE(t->elfPageCount == 1);

  Scheduler::exec(0);

  // `exec()` failed at module index check, before touching `elfPageCount`. `elfPageCount` should
  // remain unchanged since we never entered the cleanup path.
  CHECK(t->elfPageCount == 1);
}

TEST_CASE_FIXTURE(ExecFixture, "exec: currentIdx out of range returns -1")
{
  SchedulerTestAccess::setCurrentIdx(-1);
  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));

  SchedulerTestAccess::setCurrentIdx(Scheduler::MAX_TASKS);
  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ExecFixture, "exec: non-zero currentIdx on ring-0 task returns -1")
{
  // Slot 0 is always idle (ring-0).
  SchedulerTestAccess::setCurrentIdx(0);
  CHECK(Scheduler::exec(0) == static_cast<uint32_t>(-1));
}
