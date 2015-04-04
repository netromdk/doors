#ifndef ASSERT_H
#define ASSERT_H

#include <stdlib.h>

#ifdef NDEBUG
  #define assert(exp) ((void) 0);
#else
  #define assert(exp) if (!(exp)) { __assertFail(#exp, __FILE__, __LINE__); }
#endif

void __assertFail(const char *exp, const char *file, int line);

#endif // ASSERT_H
