#include <algorithm.h>
#include <doctest/doctest.h>

namespace {

class Test {
public:
  Test(int value) : value(value)
  {
  }

  bool operator<(const Test &other) const
  {
    return value < other.value;
  }

  bool operator==(const Test &other) const
  {
    return value == other.value;
  }

  bool operator!=(const Test &other) const
  {
    return !(*this == other);
  }

  int value;
};

} // namespace

TEST_CASE("min")
{
  CHECK(min(1, 2) == 1);
  CHECK(min(100, 20) == 20);

  const int a = 10;
  const int b = 20;
  CHECK(&min(a, b) == &a);
  CHECK(min(a, b) == 10);

  const Test t1(100);
  const Test t2(30);
  CHECK(min(t1, t2) == t2);
  CHECK(&min(t1, t2) == &t2);
}
