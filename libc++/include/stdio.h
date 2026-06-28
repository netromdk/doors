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

inline void emitPadded(SnprintfBuf &buf, const char *str, size_t len,
                       int width, bool left, char pad)
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

template <typename T>
inline void formatRaw(SnprintfBuf & /*buf*/, T /*value*/, char /*fmt*/)
{
  assert(false);
}

template <>
inline void formatRaw(SnprintfBuf &buf, const char *value, char)
{
  buf.writestr(value);
}

template <>
inline void formatRaw(SnprintfBuf &buf, char *value, char fmt)
{
  formatRaw(buf, (const char *) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, uint32_t value, char fmt)
{
  char tmp[65];
  utos(value, tmp, fmtToBase(fmt), isupper(fmt));
  for (char *p = tmp; *p; p++) {
    buf.put(*p);
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, int value, char fmt)
{
  if (fmtIsUnsigned(fmt)) {
    formatRaw(buf, (uint32_t) value, fmt);
    return;
  }
  char tmp[65];
  itos(value, tmp, fmtToBase(fmt), isupper(fmt));
  for (char *p = tmp; *p; p++) {
    buf.put(*p);
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, unsigned char value, char fmt)
{
  if (fmtIsChar(fmt)) {
    buf.put(value);
    return;
  }
  formatRaw(buf, (uint32_t) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, char value, char fmt)
{
  if (fmtIsUnsigned(fmt)) {
    formatRaw(buf, (unsigned char) value, fmt);
  }
  else if (fmtIsChar(fmt)) {
    buf.put(value);
  }
  else {
    formatRaw(buf, (int) value, fmt);
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, int16_t value, char fmt)
{
  formatRaw(buf, (int) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, uint16_t value, char fmt)
{
  formatRaw(buf, (uint32_t) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, unsigned long value, char fmt)
{
  formatRaw(buf, (uint32_t) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, long value, char fmt)
{
  formatRaw(buf, (int) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, uint64_t value, char fmt)
{
  char tmp[65];
  ltos(value, tmp, fmtToBase(fmt), isupper(fmt));
  for (char *p = tmp; *p; p++) {
    buf.put(*p);
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, int64_t value, char fmt)
{
  if (fmtIsUnsigned(fmt)) {
    formatRaw(buf, (uint64_t) value, fmt);
    return;
  }
  char tmp[65];
  ltos(value, tmp, fmtToBase(fmt), isupper(fmt));
  for (char *p = tmp; *p; p++) {
    buf.put(*p);
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, bool value, char fmt)
{
  if (fmtIsBool(fmt)) {
    buf.writestr(value ? "true" : "false");
  }
  else {
    buf.writestr(value ? "1" : "0");
  }
}

template <>
inline void formatRaw(SnprintfBuf &buf, const void *value, char)
{
  buf.put('0');
  buf.put('x');
  const unsigned long addr = reinterpret_cast<unsigned long>(value);
  char tmp[65];
  ltos(addr, tmp, 16, false);
  for (char *p = tmp; *p; p++) {
    buf.put(*p);
  }
}

// Pointer types used with `%p`, delegate to `const void*` handler.
template <>
inline void formatRaw(SnprintfBuf &buf, const int *value, char fmt)
{
  formatRaw(buf, (const void *) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, int *value, char fmt)
{
  formatRaw(buf, (const void *) value, fmt);
}

template <>
inline void formatRaw(SnprintfBuf &buf, decltype(nullptr), char fmt)
{
  formatRaw(buf, (const void *) nullptr, fmt);
}

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

} // anonymous namespace

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
  return snprintf(s, static_cast<size_t>(-1), format, args...);
}

inline int printf(const char *format)
{
  char buf[2048];
  const int n = snprintf(buf, sizeof(buf), format);
  puts(buf);
  return n;
}

template <typename T, typename... Args>
inline int printf(const char *format, T value, Args... args)
{
  char buf[2048];
  const int n = snprintf(buf, sizeof(buf), format, value, args...);
  puts(buf);
  return n;
}

#endif // STDIO_H
