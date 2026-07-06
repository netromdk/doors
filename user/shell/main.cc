#include <span>
#include <string>
#include <string_view>

#include "Lib.h"
#include "lib/Syscall.h"

extern void main();
extern int dispatch(const span<string_view> &);

// ELF entry point.
extern "C" __attribute__((noreturn)) void _start()
{
  main();
  __builtin_unreachable();
}

void main()
{
  string line;
  for (;;) {
    printf("> ");
    line = readLine();
    if (line.empty()) {
      continue;
    }
    dispatch(tokenize(line));
  }
}
