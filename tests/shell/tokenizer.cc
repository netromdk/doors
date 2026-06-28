#include <doctest/doctest.h>
#include <kernel/Shell.h>

namespace {

constexpr int MAX_ARGS = 16;

} // namespace

TEST_CASE("single_cmd")
{
  string line = "uptime";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 1);
  CHECK(argv[0] == "uptime");
  CHECK(argv[argc].empty());
}

TEST_CASE("multi_args")
{
  string line = "echo hello world";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 3);
  CHECK(argv[0] == "echo");
  CHECK(argv[1] == "hello");
  CHECK(argv[2] == "world");
  CHECK(argv[argc].empty());
}

TEST_CASE("leading_spaces")
{
  string line = "  hello";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 1);
  CHECK(argv[0] == "hello");
}

TEST_CASE("multiple_spaces")
{
  string line = "a    b";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 2);
  CHECK(argv[0] == "a");
  CHECK(argv[1] == "b");
}

TEST_CASE("empty")
{
  string line = "";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 0);
}

TEST_CASE("whitespace_only")
{
  string line = "   ";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 0);
}

TEST_CASE("overflow")
{
  string line = "a b c d e f g h i j k l m n o p q r";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == MAX_ARGS - 1);
  CHECK(argv[argc].empty());
}

TEST_CASE("trailing_spaces")
{
  string line = "cmd   ";
  string argv[MAX_ARGS];
  int argc = Shell::tokenize(line, {argv, MAX_ARGS});
  CHECK(argc == 1);
  CHECK(argv[0] == "cmd");
}
