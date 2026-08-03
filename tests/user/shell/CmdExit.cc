#include <span>
#include <string_view>

#include "Commands.h"
#include "HostShims.h"

#include <doctest/doctest.h>

// Exit-code coverage for the real `CmdKill.cc`/`CmdTasks.cc` handlers, which are compiled into this
// module. The other commands are not and have weak stubs in `stubs.cc` always return 0.

TEST_CASE("exit code: kill usage returns EXIT_USAGE")
{
  string_view args[]{"kill"};
  CHECK(dispatch(span<string_view>(args, 1)) == EXIT_USAGE);
}

TEST_CASE("exit code: kill invalid id returns EXIT_USAGE")
{
  string_view args[]{"kill", "abc"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_USAGE);
}

TEST_CASE("exit code: kill success returns EXIT_SUCCESS")
{
  hostshim::setTaskctlKillResult(0);
  string_view args[]{"kill", "5"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_SUCCESS);
}

TEST_CASE("exit code: kill not found returns EXIT_FAILURE")
{
  hostshim::setTaskctlKillResult(-1);
  string_view args[]{"kill", "5"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_FAILURE);
}

TEST_CASE("exit code: tasks no args returns EXIT_SUCCESS")
{
  string_view args[]{"tasks"};
  CHECK(dispatch(span<string_view>(args, 1)) == EXIT_SUCCESS);
}

TEST_CASE("exit code: tasks invalid id returns EXIT_USAGE")
{
  string_view args[]{"tasks", "abc"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_USAGE);
}

TEST_CASE("exit code: tasks detail success returns EXIT_SUCCESS")
{
  hostshim::setTaskctlDetailResult(0);
  string_view args[]{"tasks", "3"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_SUCCESS);
}

TEST_CASE("exit code: tasks detail not found returns EXIT_FAILURE")
{
  hostshim::setTaskctlDetailResult(-1);
  string_view args[]{"tasks", "3"};
  CHECK(dispatch(span<string_view>(args, 2)) == EXIT_FAILURE);
}
