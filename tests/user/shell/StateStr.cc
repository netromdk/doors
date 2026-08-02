#include <cstdint>
#include <cstring>

#include "Util.h"
#include "lib/Syscall.h"

#include <doctest/doctest.h>

TEST_CASE("taskStateStr: state values")
{
  CHECK(strcmp(taskStateStr(TASK_STATE_READY), "READY") == 0);
  CHECK(strcmp(taskStateStr(TASK_STATE_RUNNING), "RUNNING") == 0);
  CHECK(strcmp(taskStateStr(TASK_STATE_BLOCKED), "BLOCKED") == 0);
}

TEST_CASE("taskStateStr: unknown state returns DEAD")
{
  CHECK(strcmp(taskStateStr(TASK_STATE_DEAD), "DEAD") == 0);
  CHECK(strcmp(taskStateStr(TASK_STATE_MAX + 1), "DEAD") == 0);
  CHECK(strcmp(taskStateStr(UINT8_MAX), "DEAD") == 0);
}

TEST_CASE("taskStateAbbrev: state values")
{
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_READY), "READY") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_RUNNING), "RUN") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_BLOCKED), "BLOCK") == 0);
}

TEST_CASE("taskStateAbbrev: unknown state returns DEAD")
{
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_DEAD), "DEAD") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_MAX + 1), "DEAD") == 0);
  CHECK(strcmp(taskStateAbbrev(UINT8_MAX), "DEAD") == 0);
}
