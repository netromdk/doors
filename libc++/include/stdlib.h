#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

void itos(int32_t num, char *str, uint8_t base = 10);
void utos(uint32_t num, char *str, uint8_t base = 10);
void ltos(uint64_t num, char *str, uint8_t base = 10);

#endif // STDLIB_H
