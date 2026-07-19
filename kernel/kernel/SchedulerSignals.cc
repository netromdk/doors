#include <cstdint>
#include <cstring>

#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>

#ifdef __IS_DOORS_KERNEL
#include <arch/i386/Paging.h>
#endif

#ifdef __IS_DOORS_KERNEL

uint32_t Scheduler::writeSignalTrampoline(Task &t, uint32_t originalUserEsp, int sigNum)
{
  // Write 16 bytes onto the userland stack at `originalUserEsp - 16`:
  //   [0]  trampoline code (8 bytes): movl $19,%eax; int $0x80; hlt
  //   [8]  trampoline address (4 bytes): address of the code above
  //   [12] signal number (4 bytes): the signal number (handler's int arg)
  //
  // newUserESP = originalUserEsp - 8 (points at trampoline addr + signal number).
  // Handler sees: [return addr = trampoline addr] [arg = signal number] (standard cdecl frame).

  const auto newUserEsp = originalUserEsp - 8;
  const auto trampolineVaddr = originalUserEsp - 16;

  // Translate userland virtual address to kernel-writable pointer.
  auto writeAt = [&](uint32_t vaddr, const uint8_t *data, uint32_t len) {
    for (int i = 0; i < t.userStackPageCount; ++i) {
      if (vaddr >= t.userStackVaddr[i] && vaddr < t.userStackVaddr[i] + Pmm::PAGE_SIZE) {
        const auto offset = vaddr - t.userStackVaddr[i];
        const auto physAddr = t.userStackPhys[i] + offset;
        auto *dst = static_cast<uint8_t *>(physToVirt(reinterpret_cast<void *>(physAddr)));
        __builtin_memcpy(dst, data, len);
        return;
      }
    }
  };

  // Trampoline code: movl $19,%eax; int $0x80; hlt
  uint8_t trampolineCode[8]{};
  trampolineCode[0] = 0xB8; // movl $19, %eax
  trampolineCode[1] = 19;
  // Bytes 2-4 already zero.
  trampolineCode[5] = 0xCD; // int $0x80
  trampolineCode[6] = 0x80;
  trampolineCode[7] = 0xF4; // hlt

  writeAt(trampolineVaddr, trampolineCode, 8);
  writeAt(trampolineVaddr + 8, reinterpret_cast<const uint8_t *>(&trampolineVaddr), 4);
  writeAt(trampolineVaddr + 12, reinterpret_cast<const uint8_t *>(&sigNum), 4);

  return newUserEsp;
}

bool Scheduler::deliverSigsegvFromException(uint32_t *frame)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return false;
  }

  Task &t = tasks_[currentIdx_];

  if (t.userStackPageCount == 0) {
    return false;
  }

  if (t.signalHandlers[Task::SIGSEGV] == nullptr) {
    return false;
  }

  // Exception frame layout (asmExcPf):
  //   frame[8]  = error_code
  //   frame[9]  = EIP (faulting instruction)
  //   frame[11] = EFLAGS
  //   frame[12] = userESP
  const uint32_t faultEip = frame[9];
  const uint32_t faultEflags = frame[11];
  const uint32_t faultUserEsp = frame[12];

  t.savedSignalEip = faultEip;
  t.savedSignalEflags = faultEflags;
  t.savedSignalEsp = faultUserEsp;

  const uint32_t newUserEsp = writeSignalTrampoline(t, faultUserEsp, Task::SIGSEGV);

  // Redirect to handler.
  frame[7] = static_cast<uint32_t>(Task::SIGSEGV); // EAX = signal number (1st arg to handler).
  frame[9] =                                       // EIP: redirect to signal handler.
    static_cast<uint32_t>(reinterpret_cast<unsigned long long>(t.signalHandlers[Task::SIGSEGV]));
  frame[11] = 0x202;      // EFLAGS: IF=1 (re-enable interrupts on return).
  frame[12] = newUserEsp; // Userland ESP: points to trampoline on userland stack.

  // Clear handler to prevent re-fault loop.
  t.signalHandlers[Task::SIGSEGV] = nullptr;

  return true;
}

#else

bool Scheduler::deliverSigsegvFromException(uint32_t *)
{
  return false;
}

#endif

void Scheduler::sendSignal(int pid, int sig)
{
  if (sig < 0 || sig >= Task::SIGNAL_MAX) {
    return;
  }

  // Find the target task by PID, skip idle (slot 0).
  int targetIdx = -1;
  for (int i = 0; i < taskCount_; ++i) {
    if (tasks_[i].pid == pid && tasks_[i].state != TaskState::DEAD && i != 0) {
      targetIdx = i;
      break;
    }
  }
  if (targetIdx < 0) {
    return;
  }

#ifdef __IS_DOORS_KERNEL
  // SIGKILL: synchronous kill, cannot be caught.
  if (sig == Task::SIGKILL) {
    if (targetIdx == currentIdx_) {
      exitCurrentTask(Task::EXIT_CODE_SIGNAL_BASE + Task::SIGKILL);
    }
    tasks_[targetIdx].exitCode = Task::EXIT_CODE_SIGNAL_BASE + Task::SIGKILL;
    killTask(targetIdx);
    return;
  }
#endif

  // Other signals: set pending bit.
  tasks_[targetIdx].pendingSignals |= (1u << sig);

  // If the target is `BLOCKED`, unblock it so the signal is delivered on the next tick.
  if (tasks_[targetIdx].state == TaskState::BLOCKED) {
    tasks_[targetIdx].state = TaskState::READY;
    tasks_[targetIdx].wakeupMs = 0;
    removeFromSleepQueue(targetIdx);
  }
}

void Scheduler::deliverPendingSignalsAtEsp(uint32_t esp)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return;
  }
  tasks_[currentIdx_].esp = esp;
  deliverPendingSignals();
}

void Scheduler::deliverPendingSignals()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return;
  }

  Task &t = tasks_[currentIdx_];

  // Only deliver to userland tasks.
  if (t.userStackPageCount == 0) {
    t.pendingSignals = 0;
    return;
  }

  if (t.pendingSignals == 0) {
    return;
  }

  // Find the lowest-numbered pending signal.
  int sigNum = -1;
  for (int i = 1; i < Task::SIGNAL_MAX; ++i) {
    if (t.pendingSignals & (1u << i)) {
      sigNum = i;
      break;
    }
  }
  if (sigNum < 0) {
    return;
  }

#ifdef __IS_DOORS_KERNEL
  if (t.signalHandlers[sigNum] != nullptr) {
    auto *frame = reinterpret_cast<uint32_t *>(t.esp);

    // Check CS ring level to see if the timer fired while in kernel mode (ring 0), the interrupt
    // frame lacks the SS/ESP dwords, so frame offsets are wrong. Defer delivery to the next tick
    // when the task is back in userland (ring 3).
    if ((frame[9] & 3) == 0) {
      return;
    }

    // Clear the pending bit now that we can safely deliver.
    t.pendingSignals &= ~(1u << sigNum);

    t.savedSignalEip = frame[8];
    t.savedSignalEflags = frame[10];
    t.savedSignalEsp = frame[11];

    const auto newUserEsp = writeSignalTrampoline(t, frame[11], sigNum);

    // Redirect to handler.
    frame[7] = static_cast<uint32_t>(sigNum); // EAX: signal number (1st arg to handler).
    frame[8] =                                // EIP: redirect to signal handler.
      static_cast<uint32_t>(reinterpret_cast<unsigned long long>(t.signalHandlers[sigNum]));
    frame[10] = 0x202;      // EFLAGS: IF=1 (re-enable interrupts on return).
    frame[11] = newUserEsp; // Userland ESP: points to trampoline on userland stack.

    return;
  }

  // No handler: terminate with signal exit code.
  t.pendingSignals &= ~(1u << sigNum);
  exitCurrentTask(Task::EXIT_CODE_SIGNAL_BASE + sigNum);
#endif
}
