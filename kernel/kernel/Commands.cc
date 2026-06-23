#include <cstdio>

#include <arch/i386/Pic.h>
#include <kernel/Cmos.h>
#include <kernel/Commands.h>
#include <kernel/Cpu.h>
#include <kernel/Heap.h>
#include <kernel/Io.h>
#include <kernel/Mem.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Shell.h>
#include <kernel/Tty.h>

namespace {

void cmdUptime(int, const string *)
{
  uint64_t total = Pit::uptimeMs();
  uint64_t sec = total / 1000;
  uint64_t ms = total % 1000;
  printf("Uptime: %u.", sec);
  if (ms < 100) {
    putchar('0');
  }
  if (ms < 10) {
    putchar('0');
  }
  printf("%u seconds\n", ms);
}

void cmdCpuInfo(int, const string *)
{
  Cpu::dump();
}

void cmdMemInfo(int, const string *)
{
  Mem::dump();
  printf("Heap free: %u bytes (largest block: %u bytes)\n", Heap::freeMem(),
         Heap::largestFreeBlock());
}

void cmdClear(int, const string *)
{
  Tty::cls();
}

void cmdHelp(int, const string *)
{
  Shell::printHelp();
}

void cmdHalt(int, const string *)
{
  printf("Halting system.\n");
  Pic::disableInt();
  for (;;) {
    __asm__("hlt");
  }
}

void cmdReboot(int, const string *)
{
  // Wait for the keyboard controller's input buffer to be empty (bit 1 of status register = 0).
  while ((Io::inb(0x64) & 0x02)) {
  }

  // It pulls the CPU RESET line low, triggering a system-wide reboot.
  // 0xFE is the "pulse reset" command on the PS/2 controller (port 0x64).
  Io::outb(0x64, 0xFE);
}

void cmdDateTime(int, const string *)
{
  Cmos::printTime();
}

void cmdEcho(int argc, const string *argv)
{
  for (int i = 1; i < argc; i++) {
    if (i > 1) {
      putchar(' ');
    }
    printf("%s", argv[i].c_str());
  }
  putchar('\n');
}

void cmdTicks(int, const string *)
{
  printf("Ticks: %u\n", Pit::uptimeMs());
}

void cmdHeap(int, const string *)
{
  printf("Heap free: %u bytes\n", Heap::freeMem());
  printf("Largest block: %u bytes\n", Heap::largestFreeBlock());
}

void cmdPanic(int, const string *)
{
  panic("triggered from shell");
}

#ifdef __IS_DOORS_UBSAN
void cmdUbsan(int argc, const string *)
{
  // Trigger signed overflow.
  int x = __INT_MAX__ + argc;
  (void) x;
}

void cmdUbsanPtr(int, const string *)
{
  // Trigger type_mismatch_v1.
  int *p = nullptr;
  *p = 0;
}
#endif

} // namespace

void initCommands()
{
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  static const Command cmds[] = {
    {.name = "uptime", .desc = "Show system uptime", .handler = cmdUptime},
    {.name = "cpuinfo", .desc = "Show CPU information", .handler = cmdCpuInfo},
    {.name = "meminfo", .desc = "Show memory information", .handler = cmdMemInfo},
    {.name = "clear", .desc = "Clear the terminal", .handler = cmdClear},
    {.name = "help", .desc = "Show this help message", .handler = cmdHelp},
    {.name = "halt", .desc = "Halt the system", .handler = cmdHalt},
    {.name = "reboot", .desc = "Reboot the system", .handler = cmdReboot},
    {.name = "datetime", .desc = "Show current date and time from CMOS", .handler = cmdDateTime},
    {.name = "echo", .desc = "Echo text back to the terminal", .handler = cmdEcho},
    {.name = "ticks", .desc = "Show raw PIT tick count", .handler = cmdTicks},
    {.name = "heap", .desc = "Show heap allocator statistics", .handler = cmdHeap},
    {.name = "panic", .desc = "Trigger a kernel panic", .handler = cmdPanic},
#ifdef __IS_DOORS_UBSAN
    {.name = "ubsan", .desc = "Trigger UBSan overflow", .handler = cmdUbsan},
    {.name = "ubsanp", .desc = "Trigger UBSan type mismatch", .handler = cmdUbsanPtr},
#endif
  };

  for (auto &cmd : cmds) {
    Shell::registerCmd(cmd);
  }
}
