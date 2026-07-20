#include <array>
#include <doctest/doctest.h>

TEST_CASE("array aggregate init")
{
  array<int, 3> a = {1, 2, 3};
  CHECK(a[0] == 1);
  CHECK(a[1] == 2);
  CHECK(a[2] == 3);
}

TEST_CASE("array value init")
{
  const array<int, 3> a{};
  CHECK(a[0] == 0);
  CHECK(a[1] == 0);
  CHECK(a[2] == 0);
}

TEST_CASE("array size empty max_size")
{
  const array<int, 3> a{};
  CHECK(a.size() == 3);
  CHECK(a.max_size() == 3);
  CHECK(!a.empty());

  const array<int, 0> b{};
  CHECK(b.size() == 0);
  CHECK(b.max_size() == 0);
  CHECK(b.empty());
}

TEST_CASE("array front back")
{
  array<int, 3> a = {10, 20, 30};
  CHECK(a.front() == 10);
  CHECK(a.back() == 30);
}

TEST_CASE("array front back const")
{
  const array<int, 3> a = {10, 20, 30};
  CHECK(a.front() == 10);
  CHECK(a.back() == 30);
}

TEST_CASE("array data")
{
  array<int, 3> a = {1, 2, 3};
  CHECK(a.data()[0] == 1);
  CHECK(a.data()[1] == 2);
  CHECK(a.data()[2] == 3);
}

TEST_CASE("array data const")
{
  const array<int, 3> a = {1, 2, 3};
  CHECK(a.data()[0] == 1);
  CHECK(a.data()[1] == 2);
  CHECK(a.data()[2] == 3);
}

TEST_CASE("array fill")
{
  array<int, 3> a{};
  a.fill(42);
  CHECK(a[0] == 42);
  CHECK(a[1] == 42);
  CHECK(a[2] == 42);
}

TEST_CASE("array swap")
{
  array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {4, 5, 6};
  a.swap(b);
  CHECK(a[0] == 4);
  CHECK(a[1] == 5);
  CHECK(a[2] == 6);
  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
  CHECK(b[2] == 3);
}

TEST_CASE("array free swap")
{
  array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {4, 5, 6};
  swap(a, b);
  CHECK(a[0] == 4);
  CHECK(b[0] == 1);
}

TEST_CASE("array begin end")
{
  array<int, 3> a = {10, 20, 30};
  int sum = 0;
  for (int v : a) {
    sum += v;
  }
  CHECK(sum == 60);
}

TEST_CASE("array begin end const")
{
  const array<int, 3> a = {10, 20, 30};
  int sum = 0;
  for (int v : a) {
    sum += v;
  }
  CHECK(sum == 60);
}

TEST_CASE("array ranged for by value")
{
  const array<int, 3> a = {2, 4, 6};
  int sum = 0;
  for (auto v : a) {
    sum += v;
  }
  CHECK(sum == 12);
}

TEST_CASE("array ranged for by ref")
{
  array<int, 3> a = {1, 2, 3};
  for (auto &v : a) {
    v *= 10;
  }
  CHECK(a[0] == 10);
  CHECK(a[1] == 20);
  CHECK(a[2] == 30);
}

TEST_CASE("array ranged for by const ref")
{
  const array<int, 3> a = {10, 20, 30};
  int sum = 0;
  for (const auto &v : a) {
    sum += v;
  }
  CHECK(sum == 60);
}

TEST_CASE("array ranged for by ref const array")
{
  const array<int, 3> a = {5, 6, 7};
  const auto &ca = a;
  int sum = 0;
  for (const auto &v : ca) {
    sum += v;
  }
  CHECK(sum == 18);
}

TEST_CASE("array cbegin cend")
{
  const array<int, 3> a = {1, 2, 3};
  int sum = 0;
  for (int v : a) {
    sum += v;
  }
  CHECK(sum == 6);
}

TEST_CASE("array at in bounds")
{
  array<int, 3> a = {100, 200, 300};
  CHECK(a.at(0) == 100);
  CHECK(a.at(1) == 200);
  CHECK(a.at(2) == 300);
}

TEST_CASE("array at const")
{
  const array<int, 3> a = {100, 200, 300};
  CHECK(a.at(0) == 100);
  CHECK(a.at(2) == 300);
}

TEST_CASE("array operator[]")
{
  array<int, 3> a = {7, 8, 9};
  a[1] = 99;
  CHECK(a[0] == 7);
  CHECK(a[1] == 99);
  CHECK(a[2] == 9);
}

TEST_CASE("array operator[] const")
{
  const array<int, 3> a = {7, 8, 9};
  CHECK(a[1] == 8);
}

TEST_CASE("array equality")
{
  const array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {1, 2, 3};
  const array<int, 3> c = {1, 2, 4};
  CHECK(a == b);
  CHECK(!(a == c));
}

TEST_CASE("array inequality")
{
  const array<int, 3> a = {1, 2, 3};
  const array<int, 3> b = {1, 2, 3};
  array<int, 3> c = {1, 2, 4};
  CHECK(!(a != b));
  CHECK(a != c);
}

TEST_CASE("array less")
{
  const array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {1, 2, 4};
  CHECK(a < b);
  CHECK(!(b < a));
}

TEST_CASE("array greater")
{
  array<int, 3> a = {1, 2, 3};
  const array<int, 3> b = {1, 2, 4};
  CHECK(b > a);
  CHECK(!(a > b));
}

TEST_CASE("array less equal")
{
  const array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {1, 2, 3};
  array<int, 3> c = {1, 2, 4};
  CHECK(a <= b);
  CHECK(a <= c);
  CHECK(!(c <= a));
}

TEST_CASE("array greater equal")
{
  array<int, 3> a = {1, 2, 3};
  array<int, 3> b = {1, 2, 3};
  const array<int, 3> c = {1, 2, 4};
  CHECK(a >= b);
  CHECK(c >= a);
  CHECK(!(a >= c));
}

TEST_CASE("array string type")
{
  array<char, 6> a = {'h', 'e', 'l', 'l', 'o', '\0'};
  CHECK(a[0] == 'h');
  CHECK(a[4] == 'o');
  CHECK(a[5] == '\0');
}

TEST_CASE("array constexpr")
{
  constexpr array<int, 3> a = {10, 20, 30};
  static_assert(a[0] == 10);
  static_assert(a[1] == 20);
  static_assert(a[2] == 30);
  static_assert(a.size() == 3);
  static_assert(a.max_size() == 3);
  static_assert(!a.empty());
  static_assert(a.front() == 10);
  static_assert(a.back() == 30);
  static_assert(a.data() == &a[0]);
  static_assert(a.at(0) == 10);
  static_assert(a.at(1) == 20);
  static_assert(a.at(2) == 30);
  static_assert(*a.cbegin() == 10);
  static_assert(*(a.cend() - 1) == 30);

  constexpr array<int, 3> b = {10, 20, 30};
  constexpr array<int, 3> c = {10, 20, 31};
  static_assert(a == b);
  static_assert(!(a == c));
  static_assert(a != c);
  static_assert(!(a != b));
  static_assert(a < c);
  static_assert(!(c < a));
  static_assert(c > a);
  static_assert(!(a > c));
  static_assert(a <= b);
  static_assert(a <= c);
  static_assert(c >= a);
  static_assert(a >= b);

  constexpr array<int, 3> d = {1, 2, 3};
  static_assert(d[0] == 1);
}

TEST_CASE("array zero size")
{
  array<int, 0> a{};
  CHECK(a.empty());
  CHECK(a.size() == 0);
  CHECK(a.max_size() == 0);
  CHECK(a.begin() == a.end());
  CHECK(a.cbegin() == a.cend());
}
