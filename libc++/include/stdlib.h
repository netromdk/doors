#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void itos(int32_t num, char *str, uint8_t base = 10);
void utos(uint32_t num, char *str, uint8_t base = 10);
void ltos(uint64_t num, char *str, uint8_t base = 10);

#ifdef __cplusplus
}
#endif

#endif // STDLIB_H
