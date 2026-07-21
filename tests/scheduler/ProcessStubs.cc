#include <cstdint>
#include <string_view>

#include <kernel/Scheduler.h>

#include "SchedulerFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: addUserTask returns nullopt")
{
  const auto result = Scheduler::addUserTask("test");
  CHECK(!result);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: addUserElfTask returns nullopt")
{
  const auto result = Scheduler::addUserElfTask("test", nullptr, 0);
  CHECK(!result);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: fork returns error")
{
  const auto result = Scheduler::fork();
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: exec returns error")
{
  const auto result = Scheduler::exec(0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: waitpid returns error")
{
  int status = 0;
  const auto result = Scheduler::waitpid(&status);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SchedulerFixture, "Process Stubs: addUserElfTask with nullptr args")
{
  const auto result = Scheduler::addUserElfTask("test", nullptr, 1024);
  CHECK(!result);
}
