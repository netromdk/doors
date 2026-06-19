#include <doctest/doctest.h>
#include <kernel/Shell.h>
#include <string.h>

namespace {

constexpr int MAX_ARGS = 16;

} // namespace

TEST_CASE("single_cmd")
{
  char line[] = "uptime";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 1);
  CHECK(strcmp(argv[0], "uptime") == 0);
  CHECK(argv[argc] == nullptr);
}

TEST_CASE("multi_args")
{
  char line[] = "echo hello world";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 3);
  CHECK(strcmp(argv[0], "echo") == 0);
  CHECK(strcmp(argv[1], "hello") == 0);
  CHECK(strcmp(argv[2], "world") == 0);
  CHECK(argv[argc] == nullptr);
}

TEST_CASE("leading_spaces")
{
  char line[] = "  hello";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 1);
  CHECK(strcmp(argv[0], "hello") == 0);
}

TEST_CASE("multiple_spaces")
{
  char line[] = "a    b";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 2);
  CHECK(strcmp(argv[0], "a") == 0);
  CHECK(strcmp(argv[1], "b") == 0);
}

TEST_CASE("empty")
{
  char line[] = "";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 0);
}

TEST_CASE("whitespace_only")
{
  char line[] = "   ";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 0);
}

TEST_CASE("overflow")
{
  char line[] = "a b c d e f g h i j k l m n o p q r";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == MAX_ARGS - 1);
  CHECK(argv[argc] == nullptr);
}

TEST_CASE("trailing_spaces")
{
  char line[] = "cmd   ";
  char *argv[MAX_ARGS];
  int argc = Shell::tokenize(line, argv, MAX_ARGS);
  CHECK(argc == 1);
  CHECK(strcmp(argv[0], "cmd") == 0);
}
