#include "SchedulerFixture.h"
#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: quantumStartMs starts at 0")
{
  CHECK(SchedulerTestAccess::quantumStartMs() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: lastTickMs starts at 0")
{
  CHECK(SchedulerTestAccess::lastTickMs() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: runtimeMs accounts for elapsed time")
{
  pitTicks = 50;
  SchedulerTestAccess::setLastTickMs(0);
  Scheduler::tick(0);

  const auto *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);
  CHECK(t->runtimeMs == 50);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: runtimeMs charges correctly across multiple ticks")
{
  pitTicks = 0;
  SchedulerTestAccess::setLastTickMs(0);

  pitTicks = 30;
  Scheduler::tick(0);

  pitTicks = 80;
  Scheduler::tick(0);

  const auto *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);
  CHECK(t->runtimeMs == 80);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: programNextTick uses sleep deadline")
{
  Scheduler::addTask("sleeper", nullptr);

  // Set up a sleeper with deadline at `pitTicks + 30` which is within `PIT_MAX_MS`.
  pitTicks = 0;
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(30);

  // Advance `pitTicks` past the quantum so `programNextTick`'s quantum check is skipped and only
  // the sleep deadline determines the next PIT firing.
  SchedulerTestAccess::setCurrentIdx(0);
  pitTicks = Scheduler::QUANTUM_MS;
  SchedulerTestAccess::setQuantumStartMs(0);
  SchedulerTestAccess::programNextTick();
  CHECK(Pit::deadline() == 30);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: programNextTick uses max delay when idle")
{
  // No sleepers, idle task running. Quantum is expired so `programNextTick()` uses `PIT_MAX_MS`.
  pitTicks = Scheduler::QUANTUM_MS;
  SchedulerTestAccess::setQuantumStartMs(0);
  SchedulerTestAccess::programNextTick();
  CHECK(Pit::deadline() == Scheduler::QUANTUM_MS + PIT_MAX_MS);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: programNextTick caps at PIT_MAX_MS")
{
  Scheduler::addTask("sleeper", nullptr);

  // Set up a sleeper with a deadline far in the future.
  pitTicks = 0;
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(10000);

  // Advance `pitTicks` past the quantum so the sleep deadline is capped at `PIT_MAX_MS`.
  SchedulerTestAccess::setCurrentIdx(0);
  pitTicks = Scheduler::QUANTUM_MS;
  SchedulerTestAccess::setQuantumStartMs(0);
  SchedulerTestAccess::programNextTick();
  CHECK(Pit::deadline() == Scheduler::QUANTUM_MS + PIT_MAX_MS);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: sleep reprograms PIT for sooner deadline")
{
  // Program PIT for 50ms from now.
  pitTicks = 0;
  Pit::programForMs(50);
  CHECK(Pit::deadline() == 50);

  // Sleep for 10ms. Deadline is sooner, so PIT should be reprogrammed.
  Scheduler::sleep(10);
  CHECK(Pit::deadline() == 10);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: sleep does not reprogram PIT for later deadline")
{
  // Program PIT for 10ms from now.
  pitTicks = 0;
  Pit::programForMs(10);
  CHECK(Pit::deadline() == 10);

  // Sleep for 50ms. Deadline is later, so PIT should NOT be reprogrammed.
  Scheduler::sleep(50);
  CHECK(Pit::deadline() == 10);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: unblockTask reprograms PIT for priority")
{
  // Add a high-priority task and block it.
  Scheduler::addTask("high", nullptr);
  SchedulerTestAccess::getTask(1)->priority = Task::PRIORITY_HIGH;

  // Set current task to idle.
  SchedulerTestAccess::setCurrentIdx(0);

  // Task must be `BLOCKED` for `unblockTask()` to act on it.
  SchedulerTestAccess::getTask(1)->state = TaskState::BLOCKED;

  // Program PIT for a long delay.
  pitTicks = 0;
  Pit::programForMs(50);
  CHECK(Pit::deadline() == 50);

  // Unblock the high-priority task. Since its priority < current task priority, PIT should be
  // reprogrammed for 1ms.
  Scheduler::unblockTask(1);
  CHECK(Pit::deadline() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tickless: switchTo sets quantumStartMs")
{
  Scheduler::addTask("task1", nullptr);

  pitTicks = 100;
  SchedulerTestAccess::programNextTick();

  // Exhaust quantum to trigger a switch.
  pitTicks = Scheduler::QUANTUM_MS;
  SchedulerTestAccess::advancePit();
  Scheduler::tick(0x1000);
  CHECK(pitTicks == Scheduler::QUANTUM_MS + 1);

  CHECK(Scheduler::currentTaskId() == 1);
  CHECK(SchedulerTestAccess::quantumStartMs() == pitTicks);
}

TEST_CASE_FIXTURE(SchedulerFixture,
                  "tickless: programNextTick uses remaining quantum when not expired")
{
  Scheduler::addTask("sleeper", nullptr);

  // Sleeper deadline far in the future so sleep deadline > `PIT_MAX_MS`.
  pitTicks = 0;
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(10000);

  // Set `quantumStartMs` low enough.
  pitTicks = Scheduler::QUANTUM_MS;
  const uint64_t low = 5;
  SchedulerTestAccess::setQuantumStartMs(low);
  SchedulerTestAccess::programNextTick();

  // Remaining quantum (5ms) < sleep deadline (far future), so PIT fires at remaining.
  CHECK(pitTicks + low == Scheduler::QUANTUM_MS + low);
  CHECK(Pit::deadline() == pitTicks + low);
}
