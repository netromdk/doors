#ifndef STDIO_H
#define STDIO_H

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif
  
int putchar(int ic);
int puts(const char *str);
int printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // STDIO_H
