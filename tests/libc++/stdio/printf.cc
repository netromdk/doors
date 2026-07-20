#include <doctest/doctest.h>
#include <stdio.h>

TEST_CASE("printf")
{
  CHECK(printf("hello world\n") == 12);

  // hello = +5
  // \n = +1
  // = 6
  CHECK(printf("%s\n", "hello") == 6);

  // hello = +5
  // space = +1
  // world = +5
  // \n = +1
  // = 12
  CHECK(printf("hello %s\n", "world") == 12);

  // I = +1
  // space = +1
  // am = +2
  // space = +1
  // 1337 = 0x539 = +3
  // \n = +1
  // = 9
  CHECK(printf("I am %x\n", 1337) == 9);

  // Unsign = +6
  // space = +1
  // this = +4
  // space = +1
  // -1 = (unsigned) 18446744073709551615 = +20
  // \n = +1
  // = 33
  const int64_t n = -1;
  CHECK(printf("Unsign this %u\n", n) == 33);

  // (const char*) nullptr = (NULL) = +6
  // \n = +1
  // = 7
  const char *str = nullptr;
  CHECK(printf("%s\n", str) == 7);
}
