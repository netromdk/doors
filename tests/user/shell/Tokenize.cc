#include "Lib.h"

#include <doctest/doctest.h>

TEST_CASE("tokenize: empty string")
{
  const string line;
  const auto r = tokenize(line);
  CHECK(r.empty());
}

TEST_CASE("tokenize: single word")
{
  const string line = "hello";
  const auto r = tokenize(line);
  CHECK(r.size() == 1);
  CHECK(r[0] == "hello");
}

TEST_CASE("tokenize: multiple words")
{
  const string line = "  foo   bar\tbaz\nqux  ";
  const auto r = tokenize(line);
  CHECK(r.size() == 4);
  CHECK(r[0] == "foo");
  CHECK(r[1] == "bar");
  CHECK(r[2] == "baz");
  CHECK(r[3] == "qux");
}

TEST_CASE("tokenize: leading and trailing whitespace")
{
  const string line = "  \t\n  cmd  ";
  const auto r = tokenize(line);
  CHECK(r.size() == 1);
  CHECK(r[0] == "cmd");
}

TEST_CASE("tokenize: limit")
{
  const string line = "a b c d e";
  const auto r = tokenize(line);
  CHECK(r.size() == 5);
  CHECK(r[0] == "a");
  CHECK(r[1] == "b");
  CHECK(r[2] == "c");
  CHECK(r[3] == "d");
  CHECK(r[4] == "e");
}

TEST_CASE("tokenize: tab and newline delimiters")
{
  const string line = "a\tb\nc";
  const auto r = tokenize(line);
  CHECK(r.size() == 3);
  CHECK(r[0] == "a");
  CHECK(r[1] == "b");
  CHECK(r[2] == "c");
}
