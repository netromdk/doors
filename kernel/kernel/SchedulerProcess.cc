#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>

#include <kernel/Heap.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Panic.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>

#ifdef __IS_DOORS_KERNEL
#include <arch/i386/Gdt.h>
#include <arch/i386/Paging.h>
#include <arch/i386/Pic.h>
#include <kernel/ElfLoader.h>
#endif

optional<int> Scheduler::addTaskAndBlock(string_view name, void (*entry)(), uint32_t pageDir)
{
  const InterruptGuard guard;

  // non-const for implicit move on return!
  auto id = addTaskImpl(name, entry, pageDir);
  if (!id) {
    return {};
  }

  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::addTaskAndBlock: corrupted currentIdx");
  }

  tasks_[currentIdx_].state = TaskState::BLOCKED;
  while (tasks_[currentIdx_].state == TaskState::BLOCKED) {
#ifdef __IS_DOORS_KERNEL
    // The three statements must run sequentially without a race window (if separated into three ASM
    // statements), where the compiler could schedule other instructions in between.
    __asm__("sti\n\thlt\n\tcli");
#else
    // The calling task is now BLOCKED but the scheduler is still executing on its stack.
    // This path is test-only: the loop exits so the test can continue.
    break;
#endif
  }

  return id;
}

#ifdef __IS_DOORS_KERNEL

extern "C" uint8_t userTestStart[], userTestEnd[];

namespace {

// RAII guard: unmaps a page and frees its Pmm frame on scope exit.
struct MappedFrame {
  uint32_t phys{};
  uint32_t vaddr{};
  bool owned = false;

  void dismiss()
  {
    owned = false;
  }

  ~MappedFrame()
  {
    if (owned) {
      Paging::unmapPage(vaddr);
      Pmm::freeFrame(reinterpret_cast<void *>(phys)); // NOLINT(performance-no-int-to-ptr)
    }
  }
};

// RAII guard: frees a Heap allocation on scope exit.
struct HeapAlloc {
  void *ptr = nullptr;
  bool owned = false;

  void dismiss()
  {
    owned = false;
  }

  ~HeapAlloc()
  {
    if (owned) {
      Heap::free(ptr);
    }
  }
};

bool allocAndMapUserPage(uint32_t vaddr, MappedFrame &out)
{
  void *phys = Pmm::allocFrame();
  if (!phys) {
    return false;
  }

  const auto phys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(phys));
  if (!Paging::mapPage(vaddr, phys32, PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
    Pmm::freeFrame(phys);
    return false;
  }

  out.phys = phys32;
  out.vaddr = vaddr;
  out.owned = true;
  return true;
}

bool mapUserStackPages(Task &t)
{
  for (uint32_t i = 0; i < Scheduler::USER_STACK_PAGES; ++i) {
    MappedFrame frame{};
    const uint32_t vaddr = Scheduler::USER_STACK_VADDR - i * Pmm::PAGE_SIZE;
    if (!allocAndMapUserPage(vaddr, frame)) {
      return false;
    }
    t.userStackVaddr[i] = vaddr;
    t.userStackPhys[i] = frame.phys;
    frame.dismiss();
  }
  t.userStackPageCount = Scheduler::USER_STACK_PAGES;
  return true;
}

void freePageArray(int count, const uint32_t *vaddrs, const uint32_t *phys)
{
  for (int i = 0; i < count; ++i) {
    Paging::unmapPage(vaddrs[i]);
    Pmm::freeFrame(reinterpret_cast<void *>(phys[i])); // NOLINT(performance-no-int-to-ptr)
  }
}

// Copy parent's user stack pages into the child's page directory. Each page is mapped at the same
// virtual address and the parent's contents are copied. On failure, unrolls all pages mapped so far
// and frees the child's page directory. The caller is responsible for freeing `childStack` on
// failure.
bool cloneChildStackPages(Task &child, const Task &parent)
{
  for (int i = 0; i < parent.userStackPageCount; ++i) {
    void *newPhys = Pmm::allocFrame();
    if (newPhys == nullptr) {
      freePageArray(i, child.userStackVaddr, child.userStackPhys);
      Pmm::freeFrame(reinterpret_cast<void *>(child.pageDir)); // NOLINT(performance-no-int-to-ptr)
      child.pageDir = 0;
      return false;
    }

    const auto newPhys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(newPhys));
    const auto vaddr = parent.userStackVaddr[i];

    if (!Paging::mapPage(vaddr, newPhys32, PAGE_PRESENT | PAGE_RW | PAGE_USER, child.pageDir)) {
      Pmm::freeFrame(newPhys);
      freePageArray(i, child.userStackVaddr, child.userStackPhys);
      Pmm::freeFrame(reinterpret_cast<void *>(child.pageDir)); // NOLINT(performance-no-int-to-ptr)
      child.pageDir = 0;
      return false;
    }

    const auto *src = static_cast<const uint8_t *>(physToVirt(
      reinterpret_cast<void *>(parent.userStackPhys[i]))); // NOLINT(performance-no-int-to-ptr)
    auto *dst = static_cast<uint8_t *>(
      physToVirt(reinterpret_cast<void *>(newPhys32))); // NOLINT(performance-no-int-to-ptr)
    __builtin_memcpy(dst, src, Pmm::PAGE_SIZE);

    child.userStackVaddr[i] = vaddr;
    child.userStackPhys[i] = newPhys32;
  }

  child.userStackPageCount = static_cast<int>(parent.userStackPageCount);
  return true;
}

} // namespace

optional<int> Scheduler::addUserTask(string_view name)
{
  const auto slotOpt = findSlot();
  if (!slotOpt) {
    return {};
  }
  const int slot = *slotOpt;

  Task &t = tasks_[slot];
  if (t.stackBuf != nullptr) {
    Heap::free(t.stackBuf);
  }
  t = {};
  t.priority = Task::PRIORITY_NORMAL;

  HeapAlloc kstack{allocKernelStack(), true};
  if (!kstack.ptr) {
    return {};
  }

  MappedFrame codeFrame{};
  if (!allocAndMapUserPage(USER_BASE, codeFrame)) {
    return {};
  }

  if (!mapUserStackPages(t)) {
    return {};
  }

  t.pageDir = Paging::clonePageDir();

  auto *codeDst = static_cast<uint8_t *>(
    physToVirt(reinterpret_cast<void *>(codeFrame.phys))); // NOLINT(performance-no-int-to-ptr)
  const auto codeSize = static_cast<size_t>(userTestEnd - userTestStart);
  memcpy(codeDst, userTestStart, codeSize);

  t.esp = initUserStackFrame(static_cast<uint8_t *>(kstack.ptr), USER_BASE,
                             USER_STACK_VADDR + Pmm::PAGE_SIZE);
  t.entry = nullptr;
  t.state = TaskState::READY;
  t.id = static_cast<uint8_t>(slot);
  initProcessFields(t);
  t.stackBuf = static_cast<uint8_t *>(kstack.ptr);
  t.stackSize = TASK_STACK_SIZE;
  name.copy(t.name.data(), t.name.size() - 1);
  t.name[name.size()] = '\0';
  t.userCodeBuf = codeFrame.phys;

  tss.esp0 = static_cast<uint32_t>(
    reinterpret_cast<unsigned long long>(static_cast<uint8_t *>(kstack.ptr) + TASK_STACK_SIZE));
  tss.ss0 = 0x10;

  kstack.dismiss();
  codeFrame.dismiss();
  return slot;
}

optional<int> Scheduler::addUserElfTask(string_view name, const void *elfData, size_t elfSize)
{
  const auto slotOpt = findSlot();
  if (!slotOpt) {
    return {};
  }
  const int slot = *slotOpt;

  Task &t = tasks_[slot];
  if (t.stackBuf != nullptr) {
    Heap::free(t.stackBuf);
  }
  t = {};
  t.priority = Task::PRIORITY_NORMAL;

  HeapAlloc kstack{allocKernelStack(), true};
  if (!kstack.ptr) {
    return {};
  }

  if (!mapUserStackPages(t)) {
    return {};
  }

  t.pageDir = Paging::clonePageDir();
  if (t.pageDir == 0) {
    return {};
  }

  const auto loadResult = ElfLoader::load(elfData, elfSize, t.pageDir);
  if (!loadResult) {
    return {};
  }
  const uint32_t entry = loadResult->entry;

  t.esp = initUserStackFrame(static_cast<uint8_t *>(kstack.ptr), entry,
                             USER_STACK_VADDR + Pmm::PAGE_SIZE);
  t.entry = nullptr;
  t.state = TaskState::READY;
  t.id = static_cast<uint8_t>(slot);
  initProcessFields(t);
  t.stackBuf = static_cast<uint8_t *>(kstack.ptr);
  t.stackSize = TASK_STACK_SIZE;
  name.copy(t.name.data(), t.name.size() - 1);
  t.name[name.size()] = '\0';

  // Track ELF-loaded pages so they are freed on task exit.
  t.elfPageCount = loadResult->numPages;
  for (int i = 0; i < t.elfPageCount; ++i) {
    t.elfVaddr[i] = loadResult->pages[i].vaddr;
    t.elfPhys[i] = loadResult->pages[i].phys;
  }

  tss.esp0 = static_cast<uint32_t>(
    reinterpret_cast<unsigned long long>(static_cast<uint8_t *>(kstack.ptr) + TASK_STACK_SIZE));
  tss.ss0 = 0x10;

  kstack.dismiss();
  return slot;
}

uint32_t Scheduler::fork()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return static_cast<uint32_t>(-1);
  }

  Task &parent = tasks_[currentIdx_];

  // Only ring-3 user tasks can fork.
  if (parent.userStackPageCount == 0) {
    return static_cast<uint32_t>(-1);
  }

  // Must have a valid frame saved by `asmInt80`.
  if (syscallFrameEsp == 0) {
    return static_cast<uint32_t>(-1);
  }

  const auto slotOpt = findSlot();
  if (!slotOpt) {
    return static_cast<uint32_t>(-1);
  }
  const int slot = *slotOpt;

  Task &child = tasks_[slot];
  if (child.stackBuf != nullptr) {
    Heap::free(child.stackBuf);
    child.stackBuf = nullptr;
  }
  child = {};

  // Allocate kernel stack for the child (includes canary).
  auto *childStack = allocKernelStack();
  if (childStack == nullptr) {
    return static_cast<uint32_t>(-1);
  }

  // Determine frame size from parent type. The frame at `syscallFrameEsp` is the `pushal` frame (32
  // bytes) followed by the `iret` frame. For ring-3 tasks, `iret` is 20 bytes (SS, ESP, EFLAGS, CS,
  // EIP). For ring-0 tasks, it is 12 bytes (EFLAGS, CS, EIP).
  const auto frameSize = FRAME_SIZE_USER;

  // Copy the parent's register frame from the kernel stack to the child's kernel stack. The frame
  // sits at the top of the stack buffer, same layout as `initUserStackFrame()` produces.
  const auto frameOffset = TASK_STACK_SIZE - frameSize;
  __builtin_memcpy(
    childStack + frameOffset,
    reinterpret_cast<const uint8_t *>(syscallFrameEsp), // NOLINT(performance-no-int-to-ptr)
    frameSize);

  // Set child's EAX to 0, which is the fork return value in the child. The EAX slot in the `pushal`
  // frame is at offset 28 bytes from the start of the `pushal` frame (7th dword: EDI, ESI, EBP,
  // ESP, EBX, EDX, ECX, EAX).
  reinterpret_cast<uint32_t *>(childStack + frameOffset)[7] = 0;

  // Clone the parent's page directory.
  const auto parentDir = parent.pageDir != 0 ? parent.pageDir : Paging::kernelPageDirPhys();
  child.pageDir = Paging::clonePageDir(parentDir);
  if (child.pageDir == 0) {
    Heap::free(childStack);
    return static_cast<uint32_t>(-1);
  }

  if (!cloneChildStackPages(child, parent)) {
    Heap::free(childStack);
    return static_cast<uint32_t>(-1);
  }

  // Set up child task fields.
  child.esp = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(childStack + frameOffset));
  child.entry = nullptr;
  child.state = TaskState::READY;
  child.id = static_cast<uint8_t>(slot);
  child.pid = nextPid_++;
  child.ppid = parent.pid;
  child.exitCode = 0;
  child.childCount = 0;
  child.priority = parent.priority;
  child.stackBuf = childStack;
  child.stackSize = TASK_STACK_SIZE;
  strncpy(child.name.data(), "fork", child.name.size() - 1);
  child.name[child.name.size() - 1] = '\0';

  // Copy the parent's FPU state to the child. If the parent is the current FPU owner, its state is
  // in the physical registers, and not yet saved to fpuState, so save it first.
  if (parent.fpuValid) {
    if (fpuOwner_ == static_cast<int>(parent.id)) {
      Cpu::fxsave(parent.fpuState);
    }
    __builtin_memcpy(child.fpuState, parent.fpuState, sizeof(child.fpuState));
    child.fpuValid = true;
  }
  else {
    child.fpuValid = false;
  }

  // Add child to parent's children list.
  if (parent.childCount < Task::MAX_CHILDREN) {
    parent.children[parent.childCount] = child.pid;
    ++parent.childCount;
  }

  return static_cast<uint32_t>(child.pid);
}

uint32_t Scheduler::exec(int modIdx)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return static_cast<uint32_t>(-1);
  }

  if (modIdx < 0 || modIdx >= static_cast<int>(Pmm::moduleCount())) {
    return static_cast<uint32_t>(-1);
  }

  Task &t = tasks_[currentIdx_];

  // Ring-0 tasks without a user page directory cannot `exec()`.
  if (t.pageDir == 0) {
    return static_cast<uint32_t>(-1);
  }

  // The old program's FPU state is meaningless after `exec()`. Force a fresh `fninit` on first FPU
  // use.
  t.fpuValid = false;

  // Reset signal handlers to `nullptr` across exec. Pending signals are preserved so they are
  // delivered after exec returns (matches Unix semantics).
  for (int i = 0; i < Task::SIGNAL_MAX; ++i) {
    t.signalHandlers[i] = nullptr;
  }
  t.savedSignalEip = 0;
  t.savedSignalEflags = 0;
  t.savedSignalEsp = 0;

  const auto *modPtr = physToVirt(reinterpret_cast<void *>( // NOLINT(performance-no-int-to-ptr)
    static_cast<uintptr_t>(Pmm::modulePhysStart(modIdx))));
  const auto modSize = Pmm::modulePhysSize(modIdx);

  // Validate the ELF before freeing anything so the old image is not destroyed on bad input.
  if (!ElfLoader::validate(modPtr, modSize)) {
    return static_cast<uint32_t>(-1);
  }

  // Free old ELF pages.
  freePageArray(t.elfPageCount, t.elfVaddr, t.elfPhys);
  t.elfPageCount = 0;

  // Free old user stack pages.
  freePageArray(t.userStackPageCount, t.userStackVaddr, t.userStackPhys);
  t.userStackPageCount = 0;

  // Free old code page if this was a non-ELF user task (loaded via `addUserTask()`).
  if (t.userCodeBuf != 0) {
    Paging::unmapPage(USER_BASE);
    Pmm::freeFrame(reinterpret_cast<void *>(t.userCodeBuf)); // NOLINT(performance-no-int-to-ptr)
    t.userCodeBuf = 0;
  }

  // Map a fresh user stack.
  if (!mapUserStackPages(t)) {
    return static_cast<uint32_t>(-1);
  }

  // Load the new ELF into the current page directory. `ElfLoader::load()` calls `clearPageTable()`
  // internally to wipe old PDEs in the ELF address range.
  const auto loadResult = ElfLoader::load(modPtr, modSize, t.pageDir);
  if (!loadResult) {
    // ELF load failed after freeing old pages. The task is effectively dead and recovery is not
    // possible.
    freePageArray(t.userStackPageCount, t.userStackVaddr, t.userStackPhys);
    t.userStackPageCount = 0;
    return static_cast<uint32_t>(-1);
  }

  // Track new ELF pages for future cleanup.
  t.elfPageCount = loadResult->numPages;
  for (int i = 0; i < t.elfPageCount; ++i) {
    t.elfVaddr[i] = loadResult->pages[i].vaddr;
    t.elfPhys[i] = loadResult->pages[i].phys;
  }

  // Modify the `iret` frame on the current kernel stack so that when `asmInt80` returns, execution
  // resumes at the new ELF entry point with a fresh user stack.
  //
  // Kernel stack layout from `syscallFrameEsp` (set by `asmInt80`):
  //
  //   `pushal` frame (`asmInt80`):
  //     +0  EDI
  //     +4  ESI
  //     +8  EBP
  //     +12 ESP
  //     +16 EBX
  //     +20 EDX
  //     +24 ECX
  //     +28 EAX
  //
  //   `iret` frame (CPU):
  //     +32 EIP
  //     +36 CS
  //     +40 EFLAGS
  //     +44 user ESP
  //     +48 user SS
  auto *frame = reinterpret_cast<uint32_t *>(syscallFrameEsp); // NOLINT(performance-no-int-to-ptr)

  // EIP (+32/4 = 8) = ELF entry point.
  frame[8] = loadResult->entry;

  // EFLAGS (+40/4 = 10):
  //   0x202 means
  //     bit 9: IF=1 (interrupts on for PIT),
  //     bit 1: reserved=1 (required by Intel)
  frame[10] = 0x00000202;

  // User ESP (+44/4 = 11): top of new stack.
  frame[11] = USER_STACK_VADDR + Pmm::PAGE_SIZE;

  // Clear EBX (+16/4 = 4) so stale register values from the SYS_EXEC don't leak into the new
  // process. The old process's EBX may hold the module index or other argument.
  frame[4] = 0;

  return 0;
}

uint32_t Scheduler::waitpid(int *status)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    return static_cast<uint32_t>(-1);
  }

  Task &parent = tasks_[currentIdx_];

  // Validate the status pointer if provided.
#if defined(__IS_DOORS_KERNEL) && defined(__i386__)
  if (status != nullptr) {
    if (reinterpret_cast<uint32_t>(status) >= KERNEL_VIRTUAL_BASE) {
      return static_cast<uint32_t>(-1);
    }
  }
#endif

  // Try to reap a dead child immediately.
  if (const auto reaped = reapDeadChild(parent, status); reaped != static_cast<uint32_t>(-1)) {
    return reaped;
  }

  // No dead child found.
  if (parent.childCount == 0) {
    return static_cast<uint32_t>(-1);
  }

  // Set `unblockOnExit` on all non-DEAD children so the current task (the one calling `waitpid()`)
  // is woken when a child exits. This reuses the existing per-task unblock mechanism: each child
  // will call `unblockTask()` on exit.
  for (int i = 0; i < parent.childCount; ++i) {
    const auto childPid = parent.children[i];
    for (int t = 0; t < taskCount_; ++t) {
      if (tasks_[t].pid == childPid && tasks_[t].state != TaskState::DEAD) {
        tasks_[t].unblockOnExit = static_cast<int8_t>(currentIdx_);
      }
    }
  }

  blockCurrentTaskAndYield();

  // After being unblocked, a child has exited. Try to reap again.
  return reapDeadChild(parent, status);
}

#else

optional<int> Scheduler::addUserTask(string_view)
{
  return {};
}

optional<int> Scheduler::addUserElfTask(string_view, const void *, size_t)
{
  return {};
}

uint32_t Scheduler::fork()
{
  return static_cast<uint32_t>(-1);
}

uint32_t Scheduler::exec(int)
{
  return static_cast<uint32_t>(-1);
}

uint32_t Scheduler::waitpid(int *)
{
  return static_cast<uint32_t>(-1);
}

#endif

void Scheduler::handleNm()
{
#ifdef __IS_DOORS_KERNEL
  // Lazy FPU context switch: save the previous owner's FPU state and restore the current task's
  // state. Called from the #NM (Device Not Available) exception handler when CR0.TS is set and a
  // task attempts an FPU instruction.
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::handleNm: corrupted currentIdx");
  }

  // Save the previous FPU owner's state if it is still valid and not the current task.
  if (fpuOwner_ >= 0 && fpuOwner_ != currentIdx_ && fpuOwner_ < MAX_TASKS &&
      tasks_[fpuOwner_].fpuValid) {
    Cpu::fxsave(tasks_[fpuOwner_].fpuState);
  }

  // Restore or initialize the current task's FPU state.
  if (tasks_[currentIdx_].fpuValid) {
    Cpu::fxrstor(tasks_[currentIdx_].fpuState);
  }
  else {
    Cpu::fninit();
  }

  tasks_[currentIdx_].fpuValid = true;
  fpuOwner_ = currentIdx_;

  // Clear CR0.TS (bit 3 = 2^3 = 8) so the faulting FPU instruction re-executes transparently.
  Cpu::writeCr0(Cpu::readCr0() & ~0x08);
#else
  panic("Scheduler::handleNm called in test build");
#endif
}

[[noreturn]] void Scheduler::exitCurrentTask(int code)
{
  // Disable interrupts so the current task's state transition to DEAD and the CR3 switch in
  // `switchTo()` are not interrupted. This function never returns, so the InterruptGuard destructor
  // is never called.
  const InterruptGuard guard;

  // Send EOI before direct context switch. When called from `deliverPendingSignals()` inside the
  // tick ISR, the normal path through `intTick()` (which calls `sendEoi()`) is bypassed. Without
  // this, the PIC holds IRQ0 in-service, blocking future PIT interrupts. Harmless outside ISR
  // context.
#ifdef __IS_DOORS_KERNEL
  Pic::sendEoi();
#endif

  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::exitCurrentTask: corrupted currentIdx");
  }

  if (tasks_[currentIdx_].onKill) {
    tasks_[currentIdx_].onKill();
  }

  tasks_[currentIdx_].exitCode = code;
  tasks_[currentIdx_].state = TaskState::DEAD;
  ++totalExited_;

  // Remove from sleep queue if this task was sleeping on a timer.
  removeFromSleepQueue(currentIdx_);

  // Reparent children to PID 0 (idle) so they are not orphaned.
  reparentChildren(tasks_[currentIdx_].pid);

  // Free per-task history buffer.
  if (tasks_[currentIdx_].historyBuf_ != nullptr) {
    delete[] tasks_[currentIdx_].historyBuf_;
    tasks_[currentIdx_].historyBuf_ = nullptr;
  }

  // Switch to the next READY task immediately instead of waiting for the next PIT tick to expire
  // the quantum, and thus eliminating up to ~20 ms of dead time.
  const auto next = findNext();
  if (next) {
#if defined(__IS_DOORS_KERNEL) && defined(__i386__)
    // Save per-task Pmm frames before CR3 switch. After `switchTo()` loads the new page directory,
    // `currentIdx_` will point to the next task and the reference is lost.
    const auto oldPageDir = tasks_[currentIdx_].pageDir;
    const int oldUserStackCount = tasks_[currentIdx_].userStackPageCount;
    uint32_t oldUserStackVaddr[Task::USER_STACK_PAGE_MAX];
    uint32_t oldUserStackPhys[Task::USER_STACK_PAGE_MAX];
    for (int i = 0; i < oldUserStackCount; ++i) {
      oldUserStackVaddr[i] = tasks_[currentIdx_].userStackVaddr[i];
      oldUserStackPhys[i] = tasks_[currentIdx_].userStackPhys[i];
    }
    const auto oldUserCode = tasks_[currentIdx_].userCodeBuf;
    const int oldElfCount = tasks_[currentIdx_].elfPageCount;
    uint32_t oldElfVaddr[Task::ELF_PAGE_MAX];
    uint32_t oldElfPhys[Task::ELF_PAGE_MAX];
    for (int i = 0; i < oldElfCount; ++i) {
      oldElfVaddr[i] = tasks_[currentIdx_].elfVaddr[i];
      oldElfPhys[i] = tasks_[currentIdx_].elfPhys[i];
    }
    const int oldUnblockOnExit = tasks_[currentIdx_].unblockOnExit;

    tasks_[currentIdx_].pageDir = 0;
    tasks_[currentIdx_].userStackPageCount = 0;
    tasks_[currentIdx_].userCodeBuf = 0;
    tasks_[currentIdx_].elfPageCount = 0;
    tasks_[currentIdx_].unblockOnExit = -1;

    const uint32_t esp = switchTo(*next);

    if (oldPageDir != 0) {
      Pmm::freeFrame(reinterpret_cast<void *>(oldPageDir));
    }
    freePageArray(oldUserStackCount, oldUserStackVaddr, oldUserStackPhys);
    if (oldUserCode != 0) {
      Paging::unmapPage(USER_BASE);
      Pmm::freeFrame(reinterpret_cast<void *>(oldUserCode));
    }
    freePageArray(oldElfCount, oldElfVaddr, oldElfPhys);
    if (oldUnblockOnExit != -1) {
      unblockTask(oldUnblockOnExit);
    }

    // Unlike the timer ISR path (asmIntTick -> intTick -> tick -> switchTo -> %eax -> movl %esp),
    // there is no ISR frame or return chain from `exitCurrentTask()`. Switch to the new task's
    // saved register frame directly.
    __asm__("movl %0, %%esp\n\tpopal\n\tiret" : : "r"(esp));
#endif
  }

  panic("Scheduler::exitCurrentTask: no runnable task remaining");

  for (;;) {
#ifdef __IS_DOORS_KERNEL
    __asm__("sti\n\thlt");
#else
    // This is unreachable in tests. Spin forever.
#endif
  }
}

[[noreturn]] void Scheduler::killFaultingTask()
{
  printf("Scheduler: killing task %d \"%s\" due to page fault\n", currentIdx_,
         tasks_[currentIdx_].name.data());
  exitCurrentTask(Task::EXIT_CODE_SIGNAL_BASE + Task::SIGSEGV);
}

void Scheduler::killTask(int id)
{
  if (id < 0 || id >= taskCount_) {
    return;
  }

  // Cannot kill self.
  if (id == currentIdx_) {
    return;
  }

  // Cannot kill a DEAD task.
  if (tasks_[id].state == TaskState::DEAD) {
    return;
  }

  if (tasks_[id].onKill) {
    tasks_[id].onKill();
  }

  // Unblock the task waiting on this one before marking DEAD, so the wait task can be scheduled on
  // the next tick.
  const int unblockId = static_cast<unsigned char>(tasks_[id].unblockOnExit);
  tasks_[id].unblockOnExit = -1;
  if (unblockId != -1) {
    unblockTask(unblockId);
  }

  tasks_[id].state = TaskState::DEAD;
  ++totalExited_;

  // Remove from sleep queue if this task was sleeping on a timer.
  removeFromSleepQueue(id);

  // Reparent children to PID 0 (idle).
  reparentChildren(tasks_[id].pid);

  // Free memory used by task.
  if (tasks_[id].stackBuf != nullptr) {
    Heap::free(tasks_[id].stackBuf);
    tasks_[id].stackBuf = nullptr;
  }

  // Free the task's page directory if it has one. The kernel page directory (`pageDir == 0`) is
  // never freed.
#ifdef __IS_DOORS_KERNEL
  if (tasks_[id].pageDir != 0) {
    Pmm::freeFrame(
      reinterpret_cast<void *>(tasks_[id].pageDir)); // NOLINT(performance-no-int-to-ptr)
    tasks_[id].pageDir = 0;
  }

  // Free user-mode pages (ring-3 tasks).
  freePageArray(tasks_[id].userStackPageCount, tasks_[id].userStackVaddr, tasks_[id].userStackPhys);
  tasks_[id].userStackPageCount = 0;
  if (tasks_[id].userCodeBuf != 0) {
    Paging::unmapPage(USER_BASE);
    Pmm::freeFrame(
      reinterpret_cast<void *>(tasks_[id].userCodeBuf)); // NOLINT(performance-no-int-to-ptr)
    tasks_[id].userCodeBuf = 0;
  }

  // Free ring-3 ELF-loaded pages. These are tracked separately from `userCodeBuf` because an ELF
  // task may have multiple code/data segments at arbitrary virtual addresses.
  freePageArray(tasks_[id].elfPageCount, tasks_[id].elfVaddr, tasks_[id].elfPhys);
  tasks_[id].elfPageCount = 0;
#endif

  // Free per-task history buffer (SYS_READLINE).
  if (tasks_[id].historyBuf_ != nullptr) {
    delete[] tasks_[id].historyBuf_;
    tasks_[id].historyBuf_ = nullptr;
  }

  tasks_[id].stackSize = 0;
}
