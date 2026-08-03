#include <span>
#include <string_view>

#include "Commands.h"

#include <doctest/doctest.h>

TEST_CASE("dispatch: no args returns EXIT_SUCCESS")
{
  CHECK(dispatch({}) == EXIT_SUCCESS);
}

TEST_CASE("dispatch: empty arg returns EXIT_SUCCESS")
{
  string_view args[]{{}};
  CHECK(dispatch(span<string_view>(args, 1)) == EXIT_SUCCESS);
}

TEST_CASE("dispatch: known command returns EXIT_SUCCESS")
{
  string_view args[]{"help"};
  CHECK(dispatch(span<string_view>(args, 1)) == EXIT_SUCCESS);
}

TEST_CASE("dispatch: unknown command returns 127")
{
  string_view args[]{"nonexistent"};
  CHECK(dispatch(span<string_view>(args, 1)) == EXIT_UNKNOWN_COMMAND);
}

TEST_CASE("dispatch: all known commands return their exit code")
{
  for (const auto &cmd : getCmdTable()) {
    string_view args[]{cmd.name};
    const int expected = (string_view(cmd.name) == "kill") ? EXIT_USAGE : EXIT_SUCCESS;
    CHECK(dispatch(span<string_view>(args, 1)) == expected);
  }
}
