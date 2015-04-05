#ifndef STDIO_H
#define STDIO_H

#include <sys/cdefs.h>

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

int putchar(int ic);
int puts(const char *str);

/** printf **/

namespace {
  int fmtToBase(char fmt) {
    switch (fmt) {
    default:
    case 'd': // decimal
      return 10;

    case 'b': // binary
      return 2;

    case 'o': // octal
      return 8;

    case 'x': // hexadecimal
    case 'X':
      return 16;
    }
  }

  bool fmtIsUnsigned(char fmt) {
    return fmt == 'u';
  }

  bool fmtIsChar(char fmt) {
    return fmt == 'c';
  }

  bool fmtIsBool(char fmt) {
    return fmt == 'b';
  }

  template <typename T>
  inline int _printf(T /*value*/, char /*fmt*/) {
    // Implement printf specialization!
    assert(false);
    return 0;
  }

  template <>
  inline int _printf(const char *value, char /*fmt*/) {
    if (!value) {
      value = (char*) "(NULL)";
    }
    return puts(value);
  }

  template <>
  inline int _printf(char *value, char fmt) {
    return _printf((const char*) value, fmt);
  }

  template <>
  inline int _printf(uint32_t value, char fmt) {
    char buf[255];
    utos(value, buf, fmtToBase(fmt), isupper(fmt));
    return puts(buf);
  }

  template <>
  inline int _printf(int/*32_t*/ value, char fmt) {
    char buf[255];
    if (fmtIsUnsigned(fmt)) {
      return _printf((uint32_t) value, fmt);
    }
    itos(value, buf, fmtToBase(fmt), isupper(fmt));
    return puts(buf);
  }

  template <>
  inline int _printf(unsigned char value, char fmt) {
    if (fmtIsChar(fmt)) {
      return putchar(value);
    }
    return _printf((uint32_t) value, fmt);
  }

  template <>
  inline int _printf(char value, char fmt) {
    if (fmtIsUnsigned(fmt)) {
      return _printf((unsigned char) value, fmt);
    }
    else if (fmtIsChar(fmt)) {
      return putchar(value);
    }
    return _printf((int) value, fmt);
  }

  template <>
  inline int _printf(uint64_t value, char fmt) {
    char buf[255];
    ltos(value, buf, fmtToBase(fmt), isupper(fmt));
    return puts(buf);
  }

  template <>
  inline int _printf(int64_t value, char fmt) {
    char buf[255];
    if (fmtIsUnsigned(fmt)) {
      return _printf((uint64_t) value, fmt);
    }
    ltos(value, buf, fmtToBase(fmt), isupper(fmt));
    return puts(buf);
  }

  template <>
  inline int _printf(bool value, char fmt) {
    if (fmtIsBool(fmt)) {
      return puts(value ? "true" : "false");
    }
    return puts(value ? "1" : "0");
  }
}

// Base case when there are no more arguments.
inline int printf(const char *format) {
  return puts(format);
}

template <typename T, typename... Args>
inline int printf(const char *format, T value, Args... args) {
  int written = 0;
  char c;
  while ((c = *format++)) {
    if (c == '%') {
      char fmt = *format;

      // Print the current value.
      written += _printf(value, fmt);

      // Continue with the rest of the arguments.
      written += printf(format+1, args...);
      break;
    }
    else {
      putchar(c);
      written++;
    }
  }
  return written;
}

#endif // STDIO_H
