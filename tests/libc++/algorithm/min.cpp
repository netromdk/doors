#include <algorithm.h>

class Test {
public:
  Test(int value) : value(value) { }

  inline bool operator<(Test &other) {
    return value < other.value;
  }

  inline bool operator==(Test &other) {
    return value == other.value;
  }

  inline bool operator!=(Test &other) {
    return !(*this == other);
  }  

  int value;
};

int main() {
  if (min(1, 2) != 1) {
    return 1;
  }

  if (min(100, 20) != 20) {
    return 2;
  }

  int a = 10, b = 20;
  if (min<int&>(a, b) != a) {
    return 3;
  }

  Test t1(100), t2(30);
  if (min(t1, t2) != t2) {
    return 4;
  }

  return 0;
}
