#include <string.h>
#include <stdint.h>

#include <stdio.h>

void *memset(void *ptr, int value, size_t num) {
  for (size_t i = 0; i < num; i++) {
    ((uint8_t*) ptr)[i] = (uint8_t) value;
  }
  return ptr;
}
