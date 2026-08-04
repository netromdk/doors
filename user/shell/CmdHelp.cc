#include "Commands.h"
#include "Lib.h"

int cmdHelp(const span<string_view> &)
{
  printf("Commands:\n");
  const auto cmds = getCmdTable();
  for (const auto &cmd : cmds) {
    printf("  %s - %s\n", cmd.name, cmd.desc);
  }
  return EXIT_SUCCESS;
}
