#include <doctest/doctest.h>
#include <kernel/Shell.h>
#include <string.h>

namespace {

constexpr int MAX_ARGS = 16;

bool handlerCalled = false;
int handlerArgc = 0;
const char *handlerArgv0 = nullptr;
const char *handlerArgv1 = nullptr;

void testHandler(int argc, char **argv)
{
  handlerCalled = true;
  handlerArgc = argc;
  handlerArgv0 = argv[0];
  handlerArgv1 = argv[1];
}

auto dispatchLine = []<size_t N>(char *line, char *(&argv)[N]) {
  int argc = Shell::tokenize(line, argv, N);
  return Shell::dispatch(argc, argv);
};

} // namespace

TEST_CASE("match")
{
  handlerCalled = false;
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "test";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 1);
  CHECK(strcmp(handlerArgv0, "test") == 0);
}

TEST_CASE("match_with_arg")
{
  handlerCalled = false;
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "test arg1";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == true);
  CHECK(handlerCalled == true);
  CHECK(handlerArgc == 2);
  CHECK(strcmp(handlerArgv0, "test") == 0);
  CHECK(strcmp(handlerArgv1, "arg1") == 0);
}

TEST_CASE("no_match")
{
  Command cmd{"test", "a test command", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "nonexistent";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == false);
}

TEST_CASE("empty")
{
  bool result = Shell::dispatch(0, nullptr);
  CHECK(result == true);
}

TEST_CASE("case_sensitive")
{
  Command cmd{"uptime", "show uptime", testHandler};
  Shell::registerCmd(cmd);

  char line[] = "UPTIME";
  char *argv[MAX_ARGS];
  bool result = dispatchLine(line, argv);

  CHECK(result == false);
}
