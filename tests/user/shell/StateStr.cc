#include <cstring>

#include "Util.h"

#include <doctest/doctest.h>

TEST_CASE("taskStateStr: state values")
{
  CHECK(strcmp(taskStateStr(1), "READY") == 0);
  CHECK(strcmp(taskStateStr(2), "RUNNING") == 0);
  CHECK(strcmp(taskStateStr(3), "BLOCKED") == 0);
}

TEST_CASE("taskStateStr: unknown state returns DEAD")
{
  CHECK(strcmp(taskStateStr(0), "DEAD") == 0);
  CHECK(strcmp(taskStateStr(4), "DEAD") == 0);
  CHECK(strcmp(taskStateStr(255), "DEAD") == 0);
}
