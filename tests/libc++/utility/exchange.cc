#include <doctest/doctest.h>
#include <utility>

TEST_CASE("exchange int")
{
  int x = 10;
  const int old = exchange(x, 20);
  CHECK(old == 10);
  CHECK(x == 20);
}

TEST_CASE("exchange pointer")
{
  int a = 1, b = 2;
  int *p = &a;
  int *const old = exchange(p, &b);
  CHECK(old == &a);
  CHECK(p == &b);
}

TEST_CASE("exchange char")
{
  char c = 'a';
  const char old = exchange(c, 'b');
  CHECK(old == 'a');
  CHECK(c == 'b');
}

TEST_CASE("exchange self-assignment")
{
  int x = 42;
  const int old = exchange(x, x);
  CHECK(old == 42);
  CHECK(x == 42);
}

TEST_CASE("exchange constexpr")
{
  constexpr auto result = [] {
    int a = 1;
    const int b = exchange(a, 2);
    return a + b;
  }();
  CHECK(result == 3);
}
