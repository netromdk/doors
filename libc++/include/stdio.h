#ifndef STDIO_H
#define STDIO_H

#include <sys/cdefs.h>

#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

int putchar(int ic);
int puts(const char *str);

namespace detail {

struct SnprintfBuf {
  char *buf_;
  size_t cap_;
  size_t pos_ = 0;

  void put(char c)
  {
    if (pos_ < cap_) {
      buf_[pos_] = c;
    }
    pos_++;
  }

  void write(const char *s, size_t n)
  {
    for (size_t i = 0; i < n; i++) {
      put(s[i]);
    }
  }

  void writestr(const char *s)
  {
    if (!s) s = "(NULL)";
    while (*s) {
      put(*s++);
    }
  }

  void pad(size_t n, char c)
  {
    for (size_t i = 0; i < n; i++) {
      put(c);
    }
  }
};

namespace {

inline void emitPadded(SnprintfBuf &buf, const char *str, size_t len, int width, bool left,
                       char pad)
{
  if (width > 0 && static_cast<size_t>(width) > len) {
    const size_t plen = static_cast<size_t>(width) - len;
    if (left) {
      buf.write(str, len);
      buf.pad(plen, pad);
    }
    else {
      buf.pad(plen, pad);
      buf.write(str, len);
    }
  }
  else {
    buf.write(str, len);
  }
}

constexpr int fmtToBase(char fmt)
{
  switch (fmt) {
  default:
  case 'd':
  case 'p':
    return 10;
  case 'b':
    return 2;
  case 'o':
    return 8;
  case 'x':
  case 'X':
    return 16;
  }
}

constexpr bool fmtIsUnsigned(char fmt)
{
  return fmt == 'u';
}

constexpr bool fmtIsChar(char fmt)
{
  return fmt == 'c';
}

constexpr bool fmtIsBool(char fmt)
{
  return fmt == 'b';
}

} // anonymous namespace

// The following concepts work in `-ffreestanding` and rely on operand semantics rather than
// compiler built-in type traits. And they require no <type_traits>, which has not been implemented
// yet.

// `a % b` is valid for integral and enum types. It is rejected for pointer/float/string types.
template <typename T>
concept Integer = requires(T a, T b) { a % b; };

// For unsigned types, `T(-1)` wraps to the maximum representable value (> 0).
// And for signed types, `T(-1)` is -1 (< 0).

template <typename T>
concept UnsignedInteger = Integer<T> && requires(T a) { requires T(-1) > T(0); };

template <typename T>
concept SignedInteger = Integer<T> && requires(T a) { requires T(-1) < T(0); };

// Catches all integer types (signed and unsigned, any size) in one template, eliminating 12+
// explicit specializations. Implicit conversion overloads would be ambiguous across 32- (OS) /
// 64-bit (host) and require nearly as many overloads as before. Non-integer types (char, bool,
// pointers, strings) are rejected by the `requires` clause and fall through to their non-template
// overloads.
template <typename T>
  requires UnsignedInteger<T> || SignedInteger<T>
inline void formatRaw(SnprintfBuf &buf, T value, char fmt)
{
  // Unsigned integer
  if constexpr (T(-1) > T(0)) {
    if constexpr (sizeof(T) <= sizeof(uint32_t)) {
      char tmp[65];
      utos(static_cast<uint32_t>(value), tmp, fmtToBase(fmt), isupper(fmt));
      for (char *p = tmp; *p; p++) {
        buf.put(*p);
      }
    }
    else {
      char tmp[65];
      ltos(value, tmp, fmtToBase(fmt), isupper(fmt));
      for (char *p = tmp; *p; p++) {
        buf.put(*p);
      }
    }
  }

  // Signed integer.
  else {
    if (fmtIsUnsigned(fmt)) {
      if constexpr (sizeof(T) <= sizeof(uint32_t)) {
        formatRaw(buf, static_cast<uint32_t>(value), fmt);
      }
      else {
        formatRaw(buf, static_cast<uint64_t>(value), fmt);
      }
      return;
    }
    if constexpr (sizeof(T) <= sizeof(int)) {
      char tmp[65];
      itos(static_cast<int>(value), tmp, fmtToBase(fmt), isupper(fmt));
      for (char *p = tmp; *p; p++) {
        buf.put(*p);
      }
    }
    else {
      // 64-bit signed: handle negative sign manually since `ltos` takes `uint64_t`.
      bool negative = value < T(0);
      uint64_t abs_val = negative ? -static_cast<uint64_t>(value) : static_cast<uint64_t>(value);
      char tmp[65];
      char *out = tmp;
      if (negative) {
        *out++ = '-';
      }
      ltos(abs_val, out, fmtToBase(fmt), isupper(fmt));
      for (char *p = tmp; *p; p++) {
        buf.put(*p);
      }
    }
  }
}

void formatRaw(SnprintfBuf &buf, const char *value, char);
void formatRaw(SnprintfBuf &buf, char *value, char fmt);
void formatRaw(SnprintfBuf &buf, unsigned char value, char fmt);
void formatRaw(SnprintfBuf &buf, char value, char fmt);
void formatRaw(SnprintfBuf &buf, bool value, char fmt);
void formatRaw(SnprintfBuf &buf, const void *value, char);
void formatRaw(SnprintfBuf &buf, const int *value, char fmt);
void formatRaw(SnprintfBuf &buf, int *value, char fmt);
void formatRaw(SnprintfBuf &buf, decltype(nullptr), char fmt);

template <typename T>
inline void formatPut(SnprintfBuf &buf, T value, char fmt, int width, bool left, char pad)
{
  char tmp[1024];
  SnprintfBuf mbuf{tmp, sizeof(tmp)};
  formatRaw(mbuf, value, fmt);
  emitPadded(buf, tmp, mbuf.pos_, width, left, pad);
}

template <>
inline void formatPut(SnprintfBuf &buf, const char *value, char, int width, bool left, char)
{
  if (!value) value = "(NULL)";
  size_t len = 0;
  const char *p = value;
  while (*p++) {
    len++;
  }
  emitPadded(buf, value, len, width, left, ' ');
}

inline void walkFormat(SnprintfBuf &buf, const char *format)
{
  char c;
  while ((c = *format++)) {
    if (c == '%') {
      if (*format == '%') {
        format++;
        buf.put('%');
        continue;
      }
      buf.put('%');
      buf.writestr(format);
      return;
    }
    buf.put(c);
  }
}

template <typename T, typename... Args>
inline void walkFormat(SnprintfBuf &buf, const char *format, T value, Args... args)
{
  char c;
  while ((c = *format++)) {
    if (c == '%') {
      bool left = false;
      char pad = ' ';

      while (*format == '-') {
        left = true;
        format++;
      }

      // Handle %%. Output literal %, don't consume an arg.
      if (*format == '%') {
        buf.put('%');
        format++;
        continue;
      }

      if (*format == '0') {
        pad = '0';
        format++;
        while (*format == '-') {
          left = true;
          format++;
        }
      }

      int width = 0;
      while (isdigit(*format)) {
        width = width * 10 + (*format++ - '0');
      }

      // The `-` flag overrides `0`. Left-alignment always space-pads.
      if (left) {
        pad = ' ';
      }

      const char fmt = *format++;
      formatPut(buf, value, fmt, width, left, pad);
      walkFormat(buf, format, args...);
      return;
    }
    buf.put(c);
  }
}

} // namespace detail

template <typename... Args>
inline int snprintf(char *s, size_t n, const char *format, Args... args)
{
  detail::SnprintfBuf buf{s, n};
  detail::walkFormat(buf, format, args...);
  if (n > 0) {
    if (buf.pos_ < n) {
      s[buf.pos_] = '\0';
    }
    else {
      s[n - 1] = '\0';
    }
  }
  return static_cast<int>(buf.pos_);
}

template <typename... Args>
inline int sprintf(char *s, const char *format, Args... args)
{
  // Call `walkFormat()` directly instead of delegating to `snprintf()` with `SIZE_MAX`
  // (-1). `snprintf`'s overflow branch uses `s[n-1]` which triggers `-Warray-bounds` at -O2 when
  // `n` is `SIZE_MAX`, even though that branch is unreachable (`SnprintfBuf` with unlimited
  // capacity never overflows).
  detail::SnprintfBuf buf{s, static_cast<size_t>(-1)};
  detail::walkFormat(buf, format, args...);
  s[buf.pos_] = '\0';
  return static_cast<int>(buf.pos_);
}

int printf(const char *format);

template <typename T, typename... Args>
inline int printf(const char *format, T value, Args... args)
{
  char buf[2048];
  const int n = snprintf(buf, sizeof(buf), format, value, args...);
  puts(buf);
  return n;
}

#endif // STDIO_H
