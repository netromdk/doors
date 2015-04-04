#include <stdio.h>

#include <string.h> //remove

// Revisit this test when printf() has been rewritten to using
// variadic templates instead.

int main() {
  if (printf("hello world\n") != 12) {
    return 1;
  }

  /*
  // yields 7 right now but must be 6!
  // for some reason the strlen(tmp) in printf() yields 6 instead of 5..
  return printf("%s\n", "hello");
  // TODO:
  // Try implementing printf() differently with va_args?
  
  // hello = +5
  // \n = +1
  // = 6
  if (printf("%s\n", "hello") != 6) {
    return 2;
  }

  // hello = +5
  // space = +1
  // world = +5
  // \n = +1
  // = 12
  if (printf("hello %s\n", "world") != 12) {
    return 3;
  }
  */
  return 0;
}
