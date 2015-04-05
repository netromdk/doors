#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

__attribute__((__noreturn__))
void abort();

void itos(int32_t num, char *str, uint8_t base = 10, bool upcase = false);
void utos(uint32_t num, char *str, uint8_t base = 10, bool upcase = false);
void ltos(uint64_t num, char *str, uint8_t base = 10, bool upcase = false);

#endif // STDLIB_H
