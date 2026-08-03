#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

static constexpr int SNAKE_MODULE_IDX = 1;

int cmdSnake(const span<string_view> &)
{
  const int tid = sys_execmod(SNAKE_MODULE_IDX);
  if (tid < 0) {
    print("snake: not yet available\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
