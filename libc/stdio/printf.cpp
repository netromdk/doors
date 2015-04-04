#include <stdio.h>
#include <stdlib.h>

int printf(const char *format, ...) {
  char **args = (char**) &format;
  args++;

  int written = 0;
  char buf[64];
  char c;
  while ((c = *format++)) {
    if (c != '%') {
      putchar(c);
      written++;
      continue;
    }

    char *tmp; // holder of temp. strings
    c = *format++; // get format character

    switch (c) {
    case 's': // string
      tmp = *args++;
      if (!tmp) {
        tmp = (char*) "(NULL)";
      }
      written += puts(tmp);
      break;

    case 'c': // character
      putchar((char) *((int32_t*) args++));
      written++;
      break;

    case 'b': // binary
      itos(*((int32_t*) args++), buf, 2);
      written += puts(buf);
      break;

    case 'o': // octal
      itos(*((int32_t*) args++), buf, 8);
      written += puts(buf);
      break;

    case 'd': // decimal
      itos(*((int32_t*) args++), buf, 10);
      written += puts(buf);
      break;

    case 'x': // hexadecimal
      itos(*((int32_t*) args++), buf, 16);
      written += puts(buf);
      break;

    case 'u': // unsigned integer
      utos(*((uint32_t*) args++), buf);
      written += puts(buf);
      break;

    case 'l': // unsigned long integer
      ltos(*((uint64_t*) args++), buf);
      written += puts(buf);
      break;
    }
  }

  return written;
}
