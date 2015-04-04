#include <algorithm.h>

int main() {
  if (max(1, 2) != 2) {
    return 1;
  }

  if (max(100, 20) != 100) {
    return 2;
  }

  return 0;
}
