#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

__attribute__ ((__noreturn__))
void abort();

void itos(int32_t num, char *str, uint8_t base = 10, bool upcase = false);
void utos(uint32_t num, char *str, uint8_t base = 10, bool upcase = false);
void ltos(uint64_t num, char *str, uint8_t base = 10, bool upcase = false);

/**
 * Convert string to number. If no base is given then try to
 * auto-detect base.
 */
int32_t strtol(const char *str, char **endptr = nullptr, int base = 0);
uint32_t strtoul(const char *str, char **endptr = nullptr, int base = 0);
int64_t strtoll(const char *str, char **endptr = nullptr, int base = 0);
uint64_t strtoull(const char *str, char **endptr = nullptr, int base = 0);

#endif // STDLIB_H
