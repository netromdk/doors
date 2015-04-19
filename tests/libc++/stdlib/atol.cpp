#include <stdlib.h>

int main() {
  if (atol("123") != 123) {
    return 1;
  }

  if (atol("    123") != 123) {
    return 2;
  }

  // Invalid number yields zero.
  if (atol("  a  ") != 0) {
    return 3;
  }

  return 0;
}
