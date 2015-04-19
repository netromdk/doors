#include <stdlib.h>

int main() {
  if (abs(-10) != 10) {
    return 1;
  }

  if (abs(10) != 10) {
    return 2;
  }

  if (abs(0) != 0) {
    return 3;
  }

  return 0;
}
