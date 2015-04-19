#ifndef STDLIB_H
#define STDLIB_H

#include <sys/cdefs.h>
#include <stdint.h>

__attribute__ ((__noreturn__))
void abort() noexcept;

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

/**
 * Convert string to a base 10 number.
 */
int32_t atoi(const char *str);
int64_t atol(const char *str);

template <typename T>
inline T abs(T num) {
  return (num >= 0 ? num : num * -1);
}

template <typename T>
struct div_t {
  T quot;
  T rem;
};

/**
 * Integral division of numerator by denominator.
 */
template <typename T>
div_t<T> div(T numer, T denom) {
  div_t<T> res;
  res.quot = numer / denom;
  res.rem = numer % denom;
  return res;
}

#endif // STDLIB_H
