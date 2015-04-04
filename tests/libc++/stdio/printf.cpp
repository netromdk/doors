#include <stdio.h>

int main() {
  if (printf("hello world\n") != 12) {
    return 1;
  }
  
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

  // I = +1
  // space = +1
  // am = +2
  // space = +1
  // 1337 = 0x539 = +3
  // \n = +1
  // = 9
  if (printf("I am %x\n", 1337) != 9) {
    return 4;
  }

  // Unsign = +6
  // space = +1
  // this = +4
  // space = +1
  // -1 = (unsigned) 18446744073709551615 = +21
  // = 33
  int64_t n = -1;
  if (printf("Unsign this %u\n", n) != 33) {
    return 5;
  }

  return 0;
}
