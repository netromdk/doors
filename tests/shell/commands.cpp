#include <doctest/doctest.h>
#include <kernel/Shell.h>
#include <string.h>

#include "TestHelpers.h"

namespace {

constexpr int MAX_ARGS = 16;

bool handlerCalled = false;
int handlerArgc = 0;

void testHandler(int argc, char **)
{
  handlerCalled = true;
  handlerArgc = argc;
}

} // namespace

TEST_CASE("help_registered")
{
  handlerCalled = false;
  Command cmd{"help", "show help", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "help";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 1);
}

TEST_CASE("echo_registered")
{
  handlerCalled = false;
  Command cmd{"echo", "echo test", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "echo hi";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 2);
}
