#include "ShellFixture.h"
#include <doctest/doctest.h>

#include "TestHelpers.h"

namespace {

constexpr int MAX_ARGS = 16;

bool handlerCalled = false;
int handlerArgc = 0;

void testHandler(int argc, const string *)
{
  handlerCalled = true;
  handlerArgc = argc;
}

} // namespace

TEST_CASE_FIXTURE(ShellFixture, "help_registered")
{
  handlerCalled = false;
  Command cmd{"help", "show help", testHandler};
  Shell::registerCmd(cmd);

  string line = "help";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 1);
}

TEST_CASE_FIXTURE(ShellFixture, "echo_registered")
{
  handlerCalled = false;
  Command cmd{"echo", "echo test", testHandler};
  Shell::registerCmd(cmd);

  string line = "echo hi";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 2);
}

TEST_CASE_FIXTURE(ShellFixture, "panic_registered")
{
  handlerCalled = false;
  Command cmd{"panic", "trigger panic", testHandler};
  Shell::registerCmd(cmd);

  string line = "panic";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 1);
}

TEST_CASE_FIXTURE(ShellFixture, "panic_with_args")
{
  handlerCalled = false;
  Command cmd{"panic", "trigger panic", testHandler};
  Shell::registerCmd(cmd);

  string line = "panic now";
  string argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 2);
}
