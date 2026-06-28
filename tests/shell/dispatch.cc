#include <doctest/doctest.h>
#include <kernel/Shell.h>

#include "TestHelpers.h"

namespace {

constexpr int MAX_ARGS = 16;

bool handlerCalled = false;
int handlerArgc = 0;
string handlerArgv0;
string handlerArgv1;

void testHandler(int argc, const string *argv)
{
  handlerCalled = true;
  handlerArgc = argc;
  handlerArgv0 = argv[0];
  handlerArgv1 = argv[1];
}

} // namespace

TEST_CASE("match")
{
  handlerCalled = false;
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  string line = "test";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 1);
  CHECK(handlerArgv0 == "test");
}

TEST_CASE("match_with_arg")
{
  handlerCalled = false;
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  string line = "test arg1";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 2);
  CHECK(handlerArgv0 == "test");
  CHECK(handlerArgv1 == "arg1");
}

TEST_CASE("no_match")
{
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  string line = "nonexistent";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == false);
}

TEST_CASE("empty")
{
  const bool result = Shell::dispatch({});
  CHECK(result == true);
}

TEST_CASE("case_sensitive")
{
  Command cmd{"uptime", "show uptime", testHandler};
  Shell::registerCmd(cmd);

  string line = "UPTIME";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == false);
}
