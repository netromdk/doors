#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void __assertFail(const char *exp, const char *file, int line) {
  printf("Failed assertion (%s) at %s:%d\n", exp, file, line);
  abort();
}
