#include <stdio.h>
#include <stdlib.h>

int printf(const char *format, ...) {
  char **args = (char**) &format;
  args++;

  char buf[64];
  char c;
  while ((c = *format++)) {
    if (c != '%') {
      putchar(c);
      continue;
    }

    char* tmp; // holder of temp. strings
    c = *format++; // get format character

    switch (c) {
    case 's': // string
      tmp = *args++;
      if (!tmp) {
        tmp = (char*) "(NULL)";
      }
      puts(tmp);
      break;

    case 'c': // character
      putchar((char) *((int32_t*) args++));
      break;

    case 'b': // binary
      itos(*((int32_t*) args++), buf, 2);
      puts(buf);
      break;

    case 'o': // octal
      itos(*((int32_t*) args++), buf, 8);
      puts(buf);
      break;

    case 'd': // decimal
      itos(*((int32_t*) args++), buf, 10);
      puts(buf);
      break;

    case 'x': // hexadecimal
      itos(*((int32_t*) args++), buf, 16);
      puts(buf);
      break;

    case 'u': // unsigned integer
      utos(*((uint32_t*) args++), buf);
      puts(buf);
      break;
    }
  }

  // TODO: return bytes written!
  return 0;
}
