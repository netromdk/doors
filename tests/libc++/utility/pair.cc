#include <doctest/doctest.h>
#include <utility>

TEST_CASE("pair_default_initializes_members")
{
  pair<int, double> p;
  (void) p;
}

TEST_CASE("pair_value_constructor")
{
  pair<int, double> p(42, 3.14);
  CHECK(p.first == 42);
  CHECK(p.second == 3.14);
}

TEST_CASE("pair_string_view")
{
  pair<const char *, int> p("hello", 5);
  CHECK(p.first[0] == 'h');
  CHECK(p.second == 5);
}

TEST_CASE("pair_read_and_write_members")
{
  pair<int, int> p(10, 20);
  p.first = 30;
  CHECK(p.first == 30);
  CHECK(p.second == 20);
}

TEST_CASE("pair_different_types")
{
  pair<char, bool> p('x', true);
  CHECK(p.first == 'x');
  CHECK(p.second == true);
}
