#include <kernel/Runtime.h>

int __cxa_guard_acquire(__guard *g) {
  return !*g;
}

void __cxa_guard_release (__guard *g) {
  *g = 1;
}
