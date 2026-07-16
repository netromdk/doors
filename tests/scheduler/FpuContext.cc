#include "SchedulerFixture.h"
#include <cstring>
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

TEST_CASE_FIXTURE(SchedulerFixture, "fpuValid starts false for new tasks")
{
  const auto id = Scheduler::addTask("fpu_task", nullptr);
  REQUIRE(id);

  const auto *t = SchedulerTestAccess::getTask(*id);
  CHECK_FALSE(t->fpuValid);
}

TEST_CASE_FIXTURE(SchedulerFixture, "fpuState is 512 bytes for FXSAVE/FXRSTOR")
{
  const auto id = Scheduler::addTask("size_task", nullptr);
  REQUIRE(id);

  const auto *t = SchedulerTestAccess::getTask(*id);
  CHECK(sizeof(t->fpuState) == 512);
}

TEST_CASE_FIXTURE(SchedulerFixture, "fpuState is zero-initialized")
{
  const auto id = Scheduler::addTask("fpu_task", nullptr);
  REQUIRE(id);

  const auto *t = SchedulerTestAccess::getTask(*id);
  uint8_t zeros[512]{};
  CHECK(memcmp(t->fpuState, zeros, sizeof(zeros)) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "fpuOwner starts at -1")
{
  CHECK(SchedulerTestAccess::fpuOwner() == -1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick does not touch fpuValid or fpuOwner")
{
  Scheduler::addTask("task1", nullptr);

  SchedulerTestAccess::setFpuOwner(1);
  auto *t = SchedulerTestAccess::getTask(1);
  t->fpuValid = true;

  Scheduler::tick(0x1000);

  CHECK(SchedulerTestAccess::fpuOwner() == 1);
  CHECK(t->fpuValid);
}

TEST_CASE_FIXTURE(SchedulerFixture, "fpuState alignment is 16 bytes")
{
  const auto id = Scheduler::addTask("align_task", nullptr);
  REQUIRE(id);
  const auto *t = SchedulerTestAccess::getTask(*id);
  CHECK((reinterpret_cast<unsigned long>(t->fpuState) % 16) == 0);
}
