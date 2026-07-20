#include <doctest/doctest.h>
#include <optional>
#include <string>

TEST_CASE("optional default empty")
{
  optional<int> o;
  CHECK(!o.has_value());
  CHECK(!o);
}

TEST_CASE("optional nullopt empty")
{
  optional<int> o{nullopt};
  CHECK(!o.has_value());
}

TEST_CASE("optional value construction")
{
  const optional<int> o{42};
  CHECK(o.has_value());
  CHECK(*o == 42);
}

TEST_CASE("optional value access")
{
  optional<int> o{7};
  CHECK(*o == 7);
  *o = 10;
  CHECK(*o == 10);
}

TEST_CASE("optional copy construct")
{
  optional<int> a{42};
  optional<int> b{a};
  CHECK(*b == 42);
}

TEST_CASE("optional move construct")
{
  optional<string> a{string("hello")};
  optional<string> b{static_cast<optional<string> &&>(a)};
  CHECK(*b == "hello");
}

TEST_CASE("optional arrow operator")
{
  struct Point {
    int x;
    int y;
  };
  optional<Point> o{Point{3, 4}};
  CHECK(o->x == 3);
  CHECK(o->y == 4);
}

TEST_CASE("optional assign nullopt")
{
  optional<int> o{42};
  o = nullopt;
  CHECK(!o.has_value());
}

TEST_CASE("optional assign value")
{
  optional<int> o{};
  o = 99;
  CHECK(*o == 99);
}

TEST_CASE("optional assign optional")
{
  optional<int> a{10};
  optional<int> b{20};
  a = b;
  CHECK(*a == 20);
}

TEST_CASE("optional reset")
{
  optional<int> o{42};
  CHECK(o.has_value());
  o.reset();
  CHECK(!o.has_value());
}

TEST_CASE("optional value or")
{
  optional<int> o{};
  CHECK(o.value_or(5) == 5);
}

TEST_CASE("optional value or with value")
{
  optional<int> o{10};
  CHECK(o.value_or(5) == 10);
}

TEST_CASE("optional emplace")
{
  optional<int> o{};
  o.emplace(42);
  CHECK(*o == 42);
}

TEST_CASE("optional value with value")
{
  optional<int> o{7};
  CHECK(o.value() == 7);
}

TEST_CASE("optional equality same value")
{
  optional<int> a{42};
  optional<int> b{42};
  CHECK(a == b);
}

TEST_CASE("optional equality different value")
{
  optional<int> a{42};
  optional<int> b{7};
  CHECK(a != b);
}

TEST_CASE("optional equality empty vs empty")
{
  optional<int> a{};
  optional<int> b{};
  CHECK(a == b);
}

TEST_CASE("optional equality empty vs value")
{
  optional<int> a{};
  optional<int> b{42};
  CHECK(a != b);
}

TEST_CASE("optional vs nullopt")
{
  optional<int> a{};
  optional<int> b{42};
  CHECK(a == nullopt);
  CHECK(b != nullopt);
  CHECK(nullopt == a);
  CHECK(nullopt != b);
}

TEST_CASE("optional compare with T")
{
  optional<int> a{42};
  CHECK(a == 42);
  CHECK(42 == a);
  CHECK(a != 7);
  CHECK(7 != a);
}

TEST_CASE("optional string type")
{
  optional<string> o{string("hello")};
  CHECK(o.has_value());
  CHECK(*o == "hello");
}

TEST_CASE("optional with class")
{
  struct Counter {
    int n;

    Counter(int n) : n(n)
    {
    }

    Counter(const Counter &) = default;

    Counter &operator=(const Counter &) = default;

    bool operator==(const Counter &o) const
    {
      return n == o.n;
    }
  };
  optional<Counter> o{Counter{5}};
  CHECK(o->n == 5);
  CHECK((*o).n == 5);
}

TEST_CASE("optional default constructed destructor")
{
  optional<int> o;
  // Should not crash.
}

TEST_CASE("optional reset on empty")
{
  optional<int> o;
  o.reset(); // Should be a no-op.
  CHECK(!o);
}

TEST_CASE("optional assign nullopt on empty")
{
  optional<int> o;
  o = nullopt; // Should be a no-op.
  CHECK(!o);
}

TEST_CASE("optional emplace on filled")
{
  optional<string> o{string("old")};
  o.emplace("new");
  CHECK(*o == "new");
}

TEST_CASE("optional multiple emplace cycles")
{
  optional<int> o;
  o.emplace(1);
  CHECK(*o == 1);
  o.emplace(2);
  CHECK(*o == 2);
  o.emplace(3);
  CHECK(*o == 3);
}

TEST_CASE("optional copy construct from empty")
{
  optional<int> a;
  optional<int> b{a};
  CHECK(!b);
}

TEST_CASE("optional move construct from empty")
{
  optional<int> a;
  optional<int> b{static_cast<optional<int> &&>(a)};
  CHECK(!b);
}

TEST_CASE("optional copy assign from empty")
{
  optional<int> a{42};
  optional<int> b;
  a = b;
  CHECK(!a);
}

TEST_CASE("optional move assign from empty")
{
  optional<int> a{42};
  optional<int> b;
  a = static_cast<optional<int> &&>(b);
  CHECK(!a);
}

TEST_CASE("optional assign value on filled")
{
  optional<int> o{7};
  o = 99;
  CHECK(*o == 99);
}

TEST_CASE("optional assign optional empty to filled")
{
  optional<int> a{42};
  optional<int> b;
  a = b;
  CHECK(!a);
}

TEST_CASE("optional assign optional filled to empty")
{
  optional<int> a;
  optional<int> b{42};
  a = b;
  CHECK(*a == 42);
}

TEST_CASE("optional compare with T on empty")
{
  optional<int> a;
  CHECK(a != 0);
  CHECK(0 != a);
}

TEST_CASE("optional const dereference")
{
  const optional<int> o{42};
  CHECK(*o == 42);
}

TEST_CASE("optional value or different type")
{
  optional<int> o;
  CHECK(o.value_or(5L) == 5L);
}

TEST_CASE("optional value or with value different type")
{
  optional<int> o{10};
  CHECK(o.value_or(5L) == 10L);
}

TEST_CASE("optional string lifecycle")
{
  optional<string> o{string("hello world this is a long string")};
  CHECK(*o == "hello world this is a long string");

  o = string("replaced");
  CHECK(*o == "replaced");

  o.reset();
  CHECK(!o);

  o = string("reborn");
  CHECK(*o == "reborn");
}

namespace {

struct Tracker {
  static int alive;
  int id;

  Tracker(int id) : id(id)
  {
    ++alive;
  }

  Tracker(const Tracker &o) : id(o.id)
  {
    ++alive;
  }

  Tracker(Tracker &&o) noexcept : id(o.id)
  {
    o.id = -1;
    ++alive;
  }

  ~Tracker()
  {
    --alive;
  }

  Tracker &operator=(const Tracker &o)
  {
    id = o.id;
    return *this;
  }

  Tracker &operator=(Tracker &&o) noexcept
  {
    id = o.id;
    o.id = -1;
    return *this;
  }
};

int Tracker::alive = 0;

} // namespace

TEST_CASE("optional destructor tracking")
{
  Tracker::alive = 0;

  {
    optional<Tracker> o{Tracker{5}};
    CHECK(Tracker::alive == 1);
    CHECK(o->id == 5);
  }
  CHECK(Tracker::alive == 0);

  {
    optional<Tracker> o{Tracker{10}};
    CHECK(Tracker::alive == 1);

    o.reset();
    CHECK(Tracker::alive == 0);

    o.emplace(20);
    CHECK(Tracker::alive == 1);

    o.emplace(30);
    CHECK(Tracker::alive == 1);
    CHECK(o->id == 30);
  }

  CHECK(Tracker::alive == 0);
}
