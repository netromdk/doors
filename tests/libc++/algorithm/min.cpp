#include <doctest/doctest.h>
#include <algorithm.h>

class Test {
public:
  Test(int value) : value(value) { }

  inline bool operator<(const Test &other) const {
    return value < other.value;
  }

  inline bool operator==(const Test &other) const {
    return value == other.value;
  }

  inline bool operator!=(const Test &other) const {
    return !(*this == other);
  }

  int value;
};

TEST_CASE("min") {
  CHECK(min(1, 2) == 1);
  CHECK(min(100, 20) == 20);

  int a = 10, b = 20;
  CHECK(min<int&>(a, b) == a);

  Test t1(100), t2(30);
  CHECK(min(t1, t2) == t2);
}
