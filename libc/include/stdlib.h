#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

template <typename T>
inline void swap(T &a, T &b) {
  T tmp = b;
  b = a;
  a = tmp;
}

void itos(int32_t num, char *str, uint8_t base = 10);
void utos(uint32_t num, char *str, uint8_t base = 10);

#endif // STDLIB_H
