#include <cstring>

#include <doctest/doctest.h>

#include <kernel/Heap.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include <arch/i386/Paging.h>

#include "SchedulerTestAccess.h"

extern volatile uint64_t pitTicks;
extern "C" uint32_t syscallFrameEsp;

struct ForkFixture {
  alignas(16) static inline uint8_t pool[262144];

  ForkFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    SchedulerTestAccess::resetTotalExited();
    syscallFrameEsp = 0;
  }

  static inline uint8_t fakeStack[Scheduler::TASK_STACK_SIZE];

  void setupFakeFrame()
  {
    __builtin_memset(fakeStack, 0, sizeof(fakeStack));

    auto *top = reinterpret_cast<uint32_t *>(fakeStack + Scheduler::TASK_STACK_SIZE);
    top[-1] = 0x23;       // SS: ring-3 data
    top[-2] = 0xB0000000; // ESP: user stack top
    top[-3] = 0x00000202; // EFLAGS: IF=1
    top[-4] = 0x1B;       // CS: ring-3 code
    top[-5] = 0x10000000; // EIP: user entry

    syscallFrameEsp = static_cast<uint32_t>(
      reinterpret_cast<unsigned long long>(fakeStack + Scheduler::TASK_STACK_SIZE - 32));
  }

  int addRing3Task(const char *name)
  {
    const auto id = Scheduler::addTask(name, nullptr);
    REQUIRE(id);

    auto *t = SchedulerTestAccess::getTask(*id);
    t->userStackPageCount = 1;
    t->userStackVaddr[0] = 0xB0000000;
    t->userStackPhys[0] = 0x00501000;
    t->pageDir = Paging::clonePageDir();
    return *id;
  }
};

TEST_CASE_FIXTURE(ForkFixture, "fork: ring-0 task returns -1")
{
  SchedulerTestAccess::setCurrentIdx(0);
  setupFakeFrame();
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ForkFixture, "fork: syscallFrameEsp zero returns -1")
{
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ForkFixture, "fork: currentIdx out of range returns -1")
{
  setupFakeFrame();
  SchedulerTestAccess::setCurrentIdx(-1);
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));

  SchedulerTestAccess::setCurrentIdx(Scheduler::MAX_TASKS);
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ForkFixture, "fork: returns -1 when task table is full")
{
  const auto id = addRing3Task("parent");
  SchedulerTestAccess::setCurrentIdx(id);
  setupFakeFrame();

  // Fill all remaining slots.
  for (int i = 1; i < Scheduler::MAX_TASKS; ++i) {
    if (i == id) {
      continue;
    }
    Scheduler::addTask("fill", nullptr);
  }

  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(ForkFixture, "fork: userStackPageCount checked before slot allocation")
{
  // Task with userStackPageCount == 0 is ring-0, rejected before slot search.
  const auto id = Scheduler::addTask("ring0", nullptr);
  REQUIRE(id);

  SchedulerTestAccess::setCurrentIdx(*id);
  setupFakeFrame();

  const auto heapBefore = Heap::freeMem();
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));

  // No slot was allocated, so heap should be unchanged.
  CHECK(Heap::freeMem() == heapBefore);
}

TEST_CASE_FIXTURE(ForkFixture, "fork: syscallFrameEsp checked before slot allocation")
{
  const auto id = addRing3Task("t");
  SchedulerTestAccess::setCurrentIdx(id);
  // `syscallFrameEsp` is 0.

  const auto heapBefore = Heap::freeMem();
  CHECK(Scheduler::fork() == static_cast<uint32_t>(-1));
  CHECK(Heap::freeMem() == heapBefore);
}
