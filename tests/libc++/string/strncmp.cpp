#include <string.h>

int main() {
  // Compare "he" and "he".
  if (strncmp("hello", "hello", 2) != 0) {
    return 1;
  }

  // Compares using the minimum length of inputs and 200 = 5.
  if (strncmp("hello", "hello", 200) != 0) {
    return 2;
  }

  // 1 < a
  if (strncmp("1", "a", 1) != -1) {
    return 3;
  }

  // a > 1
  if (strncmp("a", "1", 1) != 1) {
    return 4;
  }

  return 0;
}
