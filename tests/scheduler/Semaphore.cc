#include "SchedulerFixture.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Semaphore.h>
#include <kernel/Task.h>

// Test subclass that exposes protected members as public accessors.
struct Sema : Semaphore {
  using Semaphore::Semaphore;

  int testCount() const
  {
    return count_;
  }

  int testWaitCount() const
  {
    return waitCount_;
  }

  int testWaiter(int idx) const
  {
    if (idx >= 0 && idx < waitCount_) {
      return waiters_[idx];
    }
    return -1;
  }

  void setWaitCount(int count)
  {
    waitCount_ = count;
  }
};

TEST_CASE("semaphore: fast path wait and signal")
{
  Sema s(1);

  CHECK(s.testCount() == 1);

  s.wait();
  CHECK(s.testCount() == 0);

  s.signal();
  CHECK(s.testCount() == 1);
}

TEST_CASE("semaphore: counting semaphore basic")
{
  Sema s(3);

  s.wait();
  CHECK(s.testCount() == 2);

  s.wait();
  CHECK(s.testCount() == 1);

  s.wait();
  CHECK(s.testCount() == 0);

  s.signal();
  CHECK(s.testCount() == 1);

  s.signal();
  CHECK(s.testCount() == 2);

  s.signal();
  CHECK(s.testCount() == 3);
}

TEST_CASE("semaphore: signal with no waiters increments count")
{
  Sema s(0);

  CHECK(s.testCount() == 0);
  s.signal();
  CHECK(s.testCount() == 1);
  s.signal();
  CHECK(s.testCount() == 2);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: wait blocks when count is zero")
{
  Sema s(0);

  s.wait();

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);
  CHECK(s.testWaitCount() == 1);
  CHECK(s.testWaiter(0) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: signal wakes waiter")
{
  Sema s(0);

  s.wait();
  CHECK(s.testWaitCount() == 1);
  CHECK(s.testWaiter(0) == 0);

  s.signal();

  CHECK(s.testWaitCount() == 0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: FIFO waiter order")
{
  Sema s(0);

  // First waiter — task 0.
  s.wait();
  CHECK(s.testWaitCount() == 1);
  CHECK(s.testWaiter(0) == 0);

  // Add task 1 and make it current.
  const int t1 = *Scheduler::addTask("t1", nullptr);
  REQUIRE(t1 == 1);
  *SchedulerTestAccess::getCurrentIdxPtr() = 1;
  SchedulerTestAccess::getTask(1)->state = TaskState::RUNNING;

  // Second waiter — task 1.
  s.wait();
  CHECK(s.testWaitCount() == 2);
  CHECK(s.testWaiter(0) == 0);
  CHECK(s.testWaiter(1) == 1);

  // Signal should wake task 0 (FIFO).
  s.signal();
  CHECK(s.testWaitCount() == 1);
  CHECK(Scheduler::taskState(0) == TaskState::READY);
  CHECK(s.testWaiter(0) == 1);

  // Signal should wake task 1.
  s.signal();
  CHECK(s.testWaitCount() == 0);
  CHECK(Scheduler::taskState(1) == TaskState::READY);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: before scheduler init does not crash")
{
  // No Scheduler::init() — early boot fallback path.
  Sema s(1);

  s.wait();
  CHECK(s.testCount() == 0);
  s.signal();
  CHECK(s.testCount() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: blockCurrentTask sets state to BLOCKED")
{
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
  Scheduler::blockCurrentTask();
  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "semaphore: wait with full waiter list does not overflow")
{
  Sema s(0);
  s.setWaitCount(Semaphore::MAX_WAITERS);

  s.wait();
  CHECK(s.testWaitCount() == Semaphore::MAX_WAITERS);
}
