#include <cassert>
#include <cstdio>
#include <cstdlib>

void __assertFail(const char *exp, const char *file, int line)
{
  printf("Failed assertion (%s) at %s:%d\n", exp, file, line);
  abort();
}
