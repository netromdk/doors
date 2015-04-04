#include <string.h>

int main() {
  if (strcmp("hello", "hello") != 0) {
    return 1;
  }

  // t < y
  if (strcmp("hi there", "hi you") != -1) {
    return 2;
  }

  // C > c
  if (strcmp("abC", "abc") != -1) {
    return 3;
  }

  return 0;
}
