#include "lib/Syscall.h"
#include "Lib.h"

extern int dispatch(int argc, char **argv);

extern "C" __attribute__((noreturn)) void _start()
{
  char line[256];
  char *argv[16];

  for (;;) {
    printf("> ");

    if (const int n = readLine(line, sizeof(line)); n <= 0) {
      continue;
    }

    const int argc = tokenize(line, argv, 16);
    dispatch(argc, argv);
  }
}
