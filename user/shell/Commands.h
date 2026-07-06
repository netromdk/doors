#ifndef USER_COMMANDS_H
#define USER_COMMANDS_H

#include <span>
#include <string_view>

// Note: `const char*` (not `string`) for `name`/`desc` because `cmdTable` has static
// storage. `string`s non-trivial ctor/dtor require C++ runtime support (`init_array`,
// `__cxa_atexit`) which is unavailable in this freestanding target.
struct Command {
  const char *name;
  const char *desc;
  void (*handler)(const span<string_view> &);
};

span<const Command> getCmdTable();

int dispatch(const span<string_view> &);

void cmdHelp(const span<string_view> &);
void cmdClear(const span<string_view> &);
void cmdHalt(const span<string_view> &);
void cmdReboot(const span<string_view> &);
void cmdPanic(const span<string_view> &);
void cmdUptime(const span<string_view> &);
void cmdTicks(const span<string_view> &);
void cmdMemInfo(const span<string_view> &);
void cmdHeap(const span<string_view> &);
void cmdDateTime(const span<string_view> &);
void cmdCpuInfo(const span<string_view> &);
void cmdEcho(const span<string_view> &);
void cmdKill(const span<string_view> &);
void cmdTasks(const span<string_view> &);
void cmdSnake(const span<string_view> &);

#endif
