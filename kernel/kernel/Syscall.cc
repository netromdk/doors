#include <cstdint>
#include <cstdio>
#include <cstring>

#include <arch/i386/Paging.h>
#include <kernel/Acpi.h>
#include <kernel/Cpu.h>
#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Scheduler.h>
#include <kernel/Serial.h>
#include <kernel/Syscall.h>
#include <kernel/Tty.h>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Cmos.h>
#include <kernel/Cpu.h>
#include <kernel/Heap.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Pmm.h>
#include <kernel/Vga.h>
#endif

namespace {

#ifdef __IS_DOORS_KERNEL
static uint16_t savedVgaBuf_[VGA_WIDTH * VGA_HEIGHT];
static uint8_t savedCursorRow_;
static uint8_t savedCursorCol_;
#endif

bool isValidUserBuf(uint32_t addr, int count)
{
  if (addr == 0) {
    return false;
  }
  if (count <= 0) {
    return false;
  }
  if (addr >= KERNEL_VIRTUAL_BASE) {
    return false;
  }

  const auto end = addr + static_cast<uint32_t>(count);
  if (end > KERNEL_VIRTUAL_BASE) {
    return false;
  }

  // Wraparound.
  if (end < addr) {
    return false;
  }
  return true;
}

uint32_t handleRead(uint32_t addr, int count)
{
  if (!isValidUserBuf(addr, count)) {
    return static_cast<uint32_t>(-1);
  }

  auto *const buf = reinterpret_cast<char *>(static_cast<uintptr_t>(addr));
  Kbd::waitForChar();
  int n = 0;
  while (n < count && Kbd::charAvail()) {
    buf[n] = Kbd::getChar();
    n++;
  }
  return static_cast<uint32_t>(n);
}

uint32_t handleWriteSerial(uint32_t addr, uint32_t len)
{
  if (!isValidUserBuf(addr, static_cast<int>(len))) {
    return 0;
  }
  auto *const buf = reinterpret_cast<const char *>(static_cast<uintptr_t>(addr));
  for (uint32_t i = 0; i < len; ++i) {
    Serial::write(buf[i]);
  }
  return len;
}

uint32_t handlePoweroff()
{
  // Attempt 1: ACPI S5 shutdown. Works on real hardware with ACPI and on QEMU `-machine
  // pc,acpi=on`.
  Io::outw(PM1_CNT_PORT, PM1_CNT_S5);

  // Attempt 2: QEMU `isa-debug-exit` device at port 0x402. No-op on real hardware.
  Io::outl(0x402, 1);

  // Attempt 3: triple fault. With QEMU `-no-reboot` this exits, and on real hardware it reboots.
#ifdef __IS_DOORS_KERNEL
  Cpu::tripleFault();
#endif

  return 0;
}

#ifdef __IS_DOORS_KERNEL

uint32_t handleWriteStr(uint32_t addr, uint32_t len)
{
  if (!isValidUserBuf(addr, static_cast<int>(len))) {
    return 0;
  }

  auto *const buf = reinterpret_cast<const char *>(static_cast<uintptr_t>(addr));
  for (uint32_t i = 0; i < len; ++i) {
    Tty::putc(buf[i]);
  }
  return len;
}

uint32_t handleReadLine(uint32_t bufAddr, uint32_t maxlen)
{
  if (!isValidUserBuf(bufAddr, static_cast<int>(maxlen))) {
    return static_cast<uint32_t>(-1);
  }
  if (maxlen < 2) {
    return static_cast<uint32_t>(-1);
  }

  auto &task = Scheduler::currentTask();

  if (task.historyBuf_ == nullptr) {
    task.historyBuf_ = new string[Task::HISTORY_MAX];
    task.historyCount_ = 0;
    task.historyHead_ = 0;
    task.historyPos_ = -1;
  }

  string line;
  HistoryCtx hctx{task.historyBuf_, Task::HISTORY_MAX, task.historyCount_, task.historyHead_,
                  &task.historyPos_};
  Kbd::readLine(line, &hctx);

  auto *const dst = reinterpret_cast<char *>(static_cast<uintptr_t>(bufAddr));
  size_t copyLen = line.size();
  if (copyLen >= maxlen) {
    copyLen = maxlen - 1;
  }
  memcpy(dst, line.data(), copyLen);
  dst[copyLen] = '\0';

  if (!line.empty() &&
      (task.historyCount_ == 0 ||
       line != task.historyBuf_[(task.historyHead_ - 1 + Task::HISTORY_MAX) % Task::HISTORY_MAX])) {
    task.historyBuf_[task.historyHead_] = line;
    task.historyHead_ = (task.historyHead_ + 1) % Task::HISTORY_MAX;
    if (task.historyCount_ < Task::HISTORY_MAX) {
      task.historyCount_++;
    }
  }
  task.historyPos_ = -1;

  return static_cast<uint32_t>(copyLen);
}

uint32_t handleExecmod(int modIdx);

uint32_t handleIoctl(uint32_t cmd, uint32_t arg)
{
  switch (static_cast<IoctlCmd>(cmd)) {
  case IOCTL_CLEAR:
    Tty::cls();
    return 0;

  case IOCTL_HALT:
    printf("Halting system.\n");
    Cpu::disableInterrupts();
    for (;;) {
      Cpu::halt();
    }

  case IOCTL_REBOOT:
    while ((Io::inb(0x64) & 0x02)) {
    }
    Io::outb(0x64, 0xFE);
    return 0;

  case IOCTL_PUT: {
    const int row = static_cast<int>((arg >> 24) & 0xFF);
    const int col = static_cast<int>((arg >> 16) & 0xFF);
    const char ch = static_cast<char>((arg >> 8) & 0xFF);
    const uint8_t color = static_cast<uint8_t>(arg & 0xFF);
    if (row >= 0 && static_cast<size_t>(row) < VGA_HEIGHT && col >= 0 &&
        static_cast<size_t>(col) < VGA_WIDTH) {
      Tty::lock();
      VGA_RAM[row * VGA_WIDTH + col] = vgaEntry(ch, color);
      Tty::unlock();
    }
    return 0;
  }

  case IOCTL_SAVESCREEN:
    // Save cursor first because `getCursor()` acquires/releases its own lock.
    {
      const auto [r, c] = Tty::getCursor();
      savedCursorRow_ = r;
      savedCursorCol_ = c;
    }
    Tty::lock();
    memcpy(savedVgaBuf_, VGA_RAM, sizeof(savedVgaBuf_));
    Tty::unlock();
    return 0;

  case IOCTL_RESTORESCREEN:
    Tty::lock();
    memcpy(VGA_RAM, savedVgaBuf_, sizeof(savedVgaBuf_));
    Tty::unlock();
    Tty::cursorSetPos(savedCursorRow_, savedCursorCol_);
    return 0;

  case IOCTL_CURSOR_HIDE:
    Tty::cursorDisable();
    return 0;

  case IOCTL_CURSOR_SHOW:
    Tty::cursorEnable();
    return 0;

  case IOCTL_POLL_KEY: {
    const auto ke = Kbd::tryReadKey();
    if (ke.key == Kbd::Key::Unknown) {
      return static_cast<uint32_t>(-1);
    }
    return (static_cast<uint32_t>(ke.key) << 8) | static_cast<uint32_t>(ke.ch);
  }

  default:
    return static_cast<uint32_t>(-1);
  }
}

uint32_t handleTaskctlCount()
{
  const auto alive = static_cast<uint32_t>(Scheduler::aliveTaskCount());
  const auto running = static_cast<uint32_t>(Scheduler::runningReadyCount());
  const auto blocked = static_cast<uint32_t>(Scheduler::blockedTaskCount());
  const auto dead = static_cast<uint32_t>(Scheduler::deadTaskCount());
  return (alive << 24) | (running << 16) | (blocked << 8) | dead;
}

uint32_t handleTaskctlList(uint32_t bufAddr, int maxEntries)
{
  if (!isValidUserBuf(bufAddr, maxEntries * static_cast<int>(sizeof(TaskEntry)))) {
    return static_cast<uint32_t>(-1);
  }

  auto *const dst = reinterpret_cast<TaskEntry *>(static_cast<uintptr_t>(bufAddr));
  int written = 0;
  for (int i = 0; i < Scheduler::taskCount() && written < maxEntries; ++i) {
    const auto st = Scheduler::taskState(i);
    if (!st || *st == TaskState::DEAD) {
      continue;
    }

    const auto nameOpt = Scheduler::taskName(i);
    dst[written].id = static_cast<uint8_t>(i);
    if (nameOpt) {
      const auto *name = *nameOpt;
      size_t len = strlen(name);
      if (len > 15) {
        len = 15;
      }
      memcpy(dst[written].name, name, len);
      dst[written].name[len] = '\0';
    }
    else {
      dst[written].name[0] = '\0';
    }

    dst[written].state = static_cast<uint8_t>(*st);
    ++written;
  }
  return static_cast<uint32_t>(written);
}

uint32_t handleTaskctlKill(uint32_t tid)
{
  const int id = static_cast<int>(tid);
  if (id < 0 || id >= Scheduler::taskCount()) {
    return static_cast<uint32_t>(-1);
  }
  if (id == Scheduler::currentTaskId()) {
    return static_cast<uint32_t>(-1);
  }

  // "idle" task is not killable.
  if (id == 0) {
    return static_cast<uint32_t>(-1);
  }

  const auto st = Scheduler::taskState(id);
  if (!st || *st == TaskState::DEAD) {
    return static_cast<uint32_t>(-1);
  }

  Scheduler::killTask(id);
  return 0;
}

uint32_t handleTaskctlDetail(uint32_t tid, uint32_t bufAddr)
{
  const int id = static_cast<int>(tid);
  if (id < 0 || id >= Scheduler::taskCount()) {
    return static_cast<uint32_t>(-1);
  }

  const auto st = Scheduler::taskState(id);
  if (!st || *st == TaskState::DEAD) {
    return static_cast<uint32_t>(-1);
  }
  if (!isValidUserBuf(bufAddr, static_cast<int>(sizeof(TaskDetail)))) {
    return static_cast<uint32_t>(-1);
  }

  auto *const dst = reinterpret_cast<TaskDetail *>(static_cast<uintptr_t>(bufAddr));
  dst->id = static_cast<uint8_t>(id);
  dst->state = static_cast<uint8_t>(*st);
  dst->flags = Scheduler::taskFlags(id).value_or(0);

  const auto name = Scheduler::taskName(id).value_or("");
  size_t nlen = strlen(name);
  if (nlen > 15) {
    nlen = 15;
  }
  memcpy(dst->name, name, nlen);

  dst->name[nlen] = '\0';
  dst->entry = static_cast<uint32_t>(Scheduler::taskEntryAddr(id).value_or(0));
  dst->stackBuf = Scheduler::taskStackBuf(id).value_or(nullptr)
                    ? reinterpret_cast<uint32_t>(*Scheduler::taskStackBuf(id))
                    : 0;
  dst->stackSize = static_cast<uint32_t>(Scheduler::taskStackSize(id).value_or(0));
  dst->runtimeMs = static_cast<uint32_t>(Scheduler::taskRuntimeMs(id).value_or(0));
  dst->esp = Scheduler::taskEsp(id).value_or(0);
  dst->wakeupMs = static_cast<uint32_t>(Scheduler::taskWakeupMs(id).value_or(0));
  return 0;
}

uint32_t handleTaskctl(uint32_t cmd, uint32_t arg1, uint32_t arg2)
{
  switch (static_cast<TaskctlCmd>(cmd)) {
  case TASKCTL_COUNT:
    return handleTaskctlCount();

  case TASKCTL_LIST:
    return handleTaskctlList(arg1, static_cast<int>(arg2));

  case TASKCTL_KILL:
    return handleTaskctlKill(arg1);

  case TASKCTL_DETAIL:
    return handleTaskctlDetail(arg1, arg2);

  default:
    return static_cast<uint32_t>(-1);
  }
}

uint32_t handleSysinfoDatetime(uint32_t bufAddr)
{
  if (!isValidUserBuf(bufAddr, static_cast<int>(sizeof(DateTimeRaw)))) {
    return static_cast<uint32_t>(-1);
  }

  auto *const dt = reinterpret_cast<DateTimeRaw *>(static_cast<uintptr_t>(bufAddr));
  Cmos::readDateTime(dt->year, dt->month, dt->day, dt->hour, dt->minute, dt->second);
  return 0;
}

uint32_t handleSysinfoCpu(uint32_t bufAddr)
{
  if (!isValidUserBuf(bufAddr, static_cast<int>(sizeof(CpuInfoRaw)))) {
    return static_cast<uint32_t>(-1);
  }

  auto *const cpu = reinterpret_cast<CpuInfoRaw *>(static_cast<uintptr_t>(bufAddr));
  Cpu::readCpuInfo(*cpu);
  return 0;
}

uint32_t handleSysinfo(uint32_t cmd, uint32_t arg)
{
  switch (static_cast<SysinfoCmd>(cmd)) {
  case SYSINFO_UPTIME:
    return static_cast<uint32_t>(Pit::uptimeMs());

  case SYSINFO_MEMFREE:
    return static_cast<uint32_t>(Heap::freeMem());

  case SYSINFO_MEMBLOCK:
    return static_cast<uint32_t>(Heap::largestFreeBlock());

  case SYSINFO_DATETIME:
    return handleSysinfoDatetime(arg);

  case SYSINFO_CPU:
    return handleSysinfoCpu(arg);

  default:
    return static_cast<uint32_t>(-1);
  }
}

uint32_t handleExecmod(int modIdx)
{
  if (modIdx < 0 || modIdx >= static_cast<int>(Pmm::moduleCount())) {
    return static_cast<uint32_t>(-1);
  }

  const void *modPtr =
    physToVirt(reinterpret_cast<void *>(static_cast<uintptr_t>(Pmm::modulePhysStart(modIdx))));
  const size_t modSize = Pmm::modulePhysSize(modIdx);
  const auto tid = Scheduler::addUserElfTask("module", modPtr, modSize);
  if (!tid) {
    return static_cast<uint32_t>(-1);
  }
  return static_cast<uint32_t>(*tid);
}

uint32_t handlePanic(uint32_t userMsgAddr)
{
  char msg[PANIC_MSG_MAX]{};
  if (userMsgAddr != 0 && userMsgAddr < KERNEL_VIRTUAL_BASE) {
    auto *const src = reinterpret_cast<const char *>(static_cast<uintptr_t>(userMsgAddr));
    uint32_t slen = 0;
    while (slen < PANIC_MSG_MAX - 1 && src[slen]) {
      ++slen;
    }
    memcpy(msg, src, slen);
    panic(msg);
  }
  else {
    panic("triggered from user task");
  }
  return 0; // unreachable
}

#endif // __IS_DOORS_KERNEL

} // namespace

extern "C" uint32_t syscallHandler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
#ifndef __IS_DOORS_KERNEL
  (void) edx;
#endif

  switch (static_cast<Syscall>(eax)) {
  case SYS_WRITE:
    Tty::putc(static_cast<char>(ebx));
    return 1;

  case SYS_EXIT:
    Scheduler::exitCurrentTask();
    return 0; // unreachable

  case SYS_READ:
    return handleRead(ebx, static_cast<int>(ecx));

  case SYS_SERIAL:
    return handleWriteSerial(ebx, ecx);

  case SYS_POWEROFF:
    return handlePoweroff();

#ifdef __IS_DOORS_KERNEL
  case SYS_WRITESTR:
    return handleWriteStr(ebx, ecx);

  case SYS_READLINE:
    return handleReadLine(ebx, ecx);

  case SYS_IOCTL:
    return handleIoctl(ebx, ecx);

  case SYS_TASKCTL:
    return handleTaskctl(ebx, ecx, edx);

  case SYS_SYSINFO:
    return handleSysinfo(ebx, ecx);

  case SYS_EXECMOD: {
    const int callerId = Scheduler::currentTaskId();
    const auto tid = handleExecmod(static_cast<int>(ebx));
    if (tid == static_cast<uint32_t>(-1)) {
      return static_cast<uint32_t>(-1);
    }
    Scheduler::setUnblockOnExit(static_cast<int>(tid), callerId);
    Scheduler::blockCurrentTaskAndYield();
    return tid;
  }

  case SYS_PANIC:
    return handlePanic(ebx);

  case SYS_SUPPRESS_TASKBAR:
    Scheduler::suppressTaskbar();
    return 0;
#endif

  default:
    return 0;
  }
}
