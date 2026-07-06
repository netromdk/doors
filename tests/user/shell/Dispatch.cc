#include <span>
#include <string_view>

#include "Commands.h"

#include <doctest/doctest.h>

TEST_CASE("dispatch: no args returns 0")
{
  CHECK(dispatch({}) == 0);
}

TEST_CASE("dispatch: empty arg returns 0")
{
  string_view args[]{{}};
  CHECK(dispatch(span<string_view>(args, 1)) == 0);
}

TEST_CASE("dispatch: known command returns 0")
{
  string_view args[]{"help"};
  CHECK(dispatch(span<string_view>(args, 1)) == 0);
}

TEST_CASE("dispatch: unknown command returns -1")
{
  string_view args[]{"nonexistent"};
  CHECK(dispatch(span<string_view>(args, 1)) == -1);
}

TEST_CASE("dispatch: all known commands return 0")
{
  for (const auto &cmd : getCmdTable()) {
    string_view args[]{cmd.name};
    CHECK(dispatch(span<string_view>(args, 1)) == 0);
  }
}
