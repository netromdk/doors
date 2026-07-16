#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <volatile.h>

#include <kernel/Heap.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>

#ifdef __IS_DOORS_KERNEL
#include <arch/i386/Gdt.h>
#include <arch/i386/Paging.h>
#include <kernel/ElfLoader.h>
#endif

array<Task, Scheduler::MAX_TASKS> Scheduler::tasks_{};
volatile int Scheduler::taskCount_{0};
volatile int Scheduler::currentIdx_{0};
volatile int Scheduler::quantumRemaining_{0};
volatile bool Scheduler::initialized_{false};
int Scheduler::totalExited_{0};
uint8_t Scheduler::nextPid_{0};
int Scheduler::fpuOwner_{-1};
array<Scheduler::SleepEntry, Scheduler::MAX_SLEEPERS> Scheduler::sleepQueue_{};
int Scheduler::sleepCount_{0};

int Scheduler::countIf(StatePred pred)
{
  return static_cast<int>(count_if(tasks_.begin(), tasks_.begin() + taskCount_,
                                   [&pred](const Task &t) { return pred(t.state); }));
}

bool Scheduler::isNotDead(TaskState s)
{
  return s != TaskState::DEAD;
}

bool Scheduler::isRunningOrReady(TaskState s)
{
  return s == TaskState::READY || s == TaskState::RUNNING;
}

bool Scheduler::isBlocked(TaskState s)
{
  return s == TaskState::BLOCKED;
}

bool Scheduler::isDead(TaskState s)
{
  return s == TaskState::DEAD;
}

void Scheduler::init()
{
  fill(tasks_.begin(), tasks_.end(), Task{});
  tasks_[0].state = TaskState::RUNNING;
  tasks_[0].id = 0;
  tasks_[0].pid = 0;
  tasks_[0].ppid = 0;
  tasks_[0].entry = nullptr;
  tasks_[0].stackBuf = nullptr;
  tasks_[0].stackSize = 0;
  strncpy(tasks_[0].name.data(), "idle", tasks_[0].name.size() - 1);
  tasks_[0].name[tasks_[0].name.size() - 1] = '\0';
  tasks_[0].priority = Task::PRIORITY_IDLE;
  taskCount_ = 1;
  currentIdx_ = 0;
  quantumRemaining_ = QUANTUM_TICKS;
  sleepCount_ = 0;
  initialized_ = true;
}

optional<int> Scheduler::addTask(string_view name, void (*entry)(), uint32_t pageDir,
                                 uint8_t priority)
{
  // Disable interrupts while modifying the shared task table so the timer ISR, which calls
  // `tick()`, does not see a partially-initialized slot.
  InterruptGuard guard;
  return addTaskImpl(name, entry, pageDir, priority);
}

optional<int> Scheduler::findSlot()
{
  const auto it = find_if(tasks_.begin(), tasks_.begin() + taskCount_,
                          [](const Task &t) { return t.state == TaskState::DEAD; });
  if (it != tasks_.begin() + taskCount_) {
    return static_cast<int>(it - tasks_.begin());
  }
  if (taskCount_ >= MAX_TASKS) {
    return {};
  }
  const int n = volatileLoad(taskCount_);
  volatileStore(taskCount_, n + 1);
  return n;
}

uint8_t *Scheduler::allocKernelStack()
{
  auto *stack = static_cast<uint8_t *>(Heap::alloc(TASK_STACK_SIZE));
  if (stack != nullptr) {
    // Canary at the base of the buffer to detect overflow.
    reinterpret_cast<uint32_t *>(stack)[0] = Task::STACK_CANARY;
  }
  return stack;
}

uint32_t Scheduler::initStackFrame(uint8_t *stack, void (*entry)())
{
  // Build the register frame that `popal; iret` will pop when the task first runs. Frame layout
  // matches what the timer ISR pushed: the top 12 bytes are consumed by `iret` (EIP -> CS ->
  // EFLAGS, popped high-to-low), and the preceding 32 bytes are consumed by `popal` (EDI -> ESI ->
  // EBP -> ESP_dummy -> EBX -> EDX -> ECX -> EAX).
  //
  // The frame sits at the TOP of the stack buffer so the task's normal stack growth (downward)
  // extends away from it, not into it.
  //
  // Offsets are negative from `stackTop` (the byte just past the allocated region):
  const auto stackTop = reinterpret_cast<uint32_t *>(stack + TASK_STACK_SIZE);

  // iret frame (12 bytes)
  stackTop[-1] = 0x00000202; // EFLAGS: IF=1 so interrupts are enabled the first time the task runs.
  stackTop[-2] = 0x08;       // CS: i386 kernel Code Segment selector.
  stackTop[-3] = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(entry)); // EIP

  // popal frame (32 bytes)
  stackTop[-4] = 0;  // EAX
  stackTop[-5] = 0;  // ECX
  stackTop[-6] = 0;  // EDX
  stackTop[-7] = 0;  // EBX
  stackTop[-8] = 0;  // ESP_dummy written by `pushal` but ignored by `popal`
  stackTop[-9] = 0;  // EBP
  stackTop[-10] = 0; // ESI
  stackTop[-11] = 0; // EDI

  return static_cast<uint32_t>(reinterpret_cast<unsigned long long>(stackTop - 11));
}

uint32_t Scheduler::initUserStackFrame(uint8_t *stack, uint32_t userEip, uint32_t userEsp)
{
  const auto stackTop = reinterpret_cast<uint32_t *>(stack + TASK_STACK_SIZE);

  // iret frame for ring 3 -> ring 0 -> ring 3 (20 bytes).
  stackTop[-1] = 0x23;       // SS: ring-3 data selector (0x20 | RPL 3)
  stackTop[-2] = userEsp;    // user ESP
  stackTop[-3] = 0x00000202; // EFLAGS: IF = 1
  stackTop[-4] = 0x1B;       // CS: ring-3 code selector (0x18 | RPL 3)
  stackTop[-5] = userEip;    // EIP: user entry point

  // pushal frame (32 bytes). All zero so every register starts at 0.
  for (int i = 6; i <= 13; ++i) {
    stackTop[-i] = 0;
  }

  return static_cast<uint32_t>(reinterpret_cast<unsigned long long>(stackTop - 13));
}

optional<int> Scheduler::addTaskImpl(string_view name, void (*entry)(), uint32_t pageDir,
                                     uint8_t priority)
{
  if (name.data() == nullptr) {
    return {};
  }

  const auto slotOpt = findSlot();
  if (!slotOpt) {
    return {};
  }
  const int slot = *slotOpt;

  Task &t = tasks_[slot];
  if (t.stackBuf != nullptr) {
    Heap::free(t.stackBuf);
    t.stackBuf = nullptr;
  }
  t.flags = 0;
  t.priority = priority;
  t.wakeupMs = 0;
  t.runtimeMs = 0;
  t.onKill = nullptr;
  t.pageDir = pageDir;

  auto *stack = allocKernelStack();
  if (stack == nullptr) {
    return {};
  }

  t.esp = initStackFrame(stack, &Scheduler::taskWrapper);
  t.entry = entry;
  t.state = TaskState::READY;
  t.id = static_cast<uint8_t>(slot);
  initProcessFields(t);
  t.stackBuf = stack;
  t.stackSize = TASK_STACK_SIZE;
  const auto len = name.copy(t.name.data(), t.name.size() - 1);
  t.name[len] = '\0';

  return slot;
}

void Scheduler::taskWrapper()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::taskWrapper: corrupted currentIdx");
  }
  if (tasks_[currentIdx_].entry) {
    tasks_[currentIdx_].entry();
  }
  exitCurrentTask(0);
}

uint32_t Scheduler::tick(uint32_t currentEsp)
{
  if (!initialized_) {
    return 0;
  }

  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::tick: corrupted currentIdx");
  }

  // Save the interrupted task's register frame pointer (`esp`).
  tasks_[currentIdx_].esp = currentEsp;

  // Detect stack overflow before it corrupts the saved frame.
  checkCanary(tasks_[currentIdx_]);

  // Charge one tick of CPU time to the running task (1 ms per PIT tick at 1000 Hz).
  ++tasks_[currentIdx_].runtimeMs;

  // Wake up BLOCKED tasks whose sleep deadline has passed.
  const uint64_t now = Pit::uptimeMs();
  for (int i = 0; i < taskCount_; ++i) {
    if (tasks_[i].state == TaskState::BLOCKED && tasks_[i].wakeupMs != 0 &&
        tasks_[i].wakeupMs <= now) {
      tasks_[i].state = TaskState::READY;
      tasks_[i].wakeupMs = 0;
    }
  }

  // Charge one tick against the current task's quantum. If quantum remains, stay.
  const int q = volatileLoad(quantumRemaining_) - 1;
  volatileStore(quantumRemaining_, q);
  if (q > 0) {
    return 0;
  }

  // Quantum expired. Find the next READY task by round-robin approach.
  const auto next = findNext();
  if (!next) {
    // No other runnable task exists. Keep running with a fresh quantum.
    quantumRemaining_ = QUANTUM_TICKS;
    return 0;
  }

  // Switch to the chosen task and return its saved `esp`.
  return switchTo(*next);
}

uint32_t Scheduler::switchTo(int next)
{
  if (tasks_[currentIdx_].state == TaskState::RUNNING) {
    tasks_[currentIdx_].state = TaskState::READY;
  }
  if (next < 0 || next >= MAX_TASKS || tasks_[next].state != TaskState::READY) {
    panic("Scheduler::switchTo: target task is not READY");
  }
  currentIdx_ = next;
  tasks_[currentIdx_].state = TaskState::RUNNING;
  quantumRemaining_ = QUANTUM_TICKS;

#ifdef __IS_DOORS_KERNEL
  // Switch to the task's page directory if it has one, otherwise use the kernel page
  // directory. Disable interrupts during CR3 load to prevent being interrupted mid-switch with a
  // half-configured address space.
  {
    InterruptGuard guard;
    const auto pd = tasks_[currentIdx_].pageDir;
    Cpu::writeCr3(pd != 0 ? pd : Paging::kernelPageDirPhys());

    // Update TSS.esp0 so that INT 0x80 from a ring-3 task switches to the correct kernel
    // stack. Ring-0 tasks (`userStackBuf == 0`) don't use the TSS for ring transitions, so leave
    // the TSS alone for them.
    if (tasks_[currentIdx_].userStackPageCount > 0) {
      tss.esp0 = static_cast<uint32_t>(
        reinterpret_cast<unsigned long long>(tasks_[currentIdx_].stackBuf + TASK_STACK_SIZE));
    }

    // Set CR0.TS (bit 3 = 2^3 = 8) so the next FPU instruction triggers #NM for lazy context
    // switching.
    Cpu::writeCr0(Cpu::readCr0() | 0x08);
  }
#endif

  return tasks_[currentIdx_].esp;
}

void Scheduler::initProcessFields(Task &t)
{
  t.pid = nextPid_++;
  t.ppid = (currentIdx_ >= 0 && currentIdx_ < MAX_TASKS) ? tasks_[currentIdx_].pid : 0;
  t.exitCode = 0;
  t.childCount = 0;
}

void Scheduler::reparentChildren(uint8_t parentId)
{
  for (int i = 0; i < taskCount_; ++i) {
    if (tasks_[i].state != TaskState::DEAD && tasks_[i].ppid == parentId) {
      tasks_[i].ppid = 0;
    }
  }
}

uint32_t Scheduler::reapDeadChild(Task &parent, int *status)
{
  for (int i = 0; i < parent.childCount; ++i) {
    const auto childPid = parent.children[i];
    for (int t = 0; t < taskCount_; ++t) {
      if (tasks_[t].pid == childPid && tasks_[t].state == TaskState::DEAD) {
        if (status != nullptr) {
          *status = tasks_[t].exitCode;
        }
        const auto result = tasks_[t].pid;
        parent.children[i] = parent.children[parent.childCount - 1];
        --parent.childCount;
        return static_cast<uint32_t>(result);
      }
    }
  }
  return static_cast<uint32_t>(-1);
}

optional<int> Scheduler::addTaskAndBlock(string_view name, void (*entry)(), uint32_t pageDir)
{
  InterruptGuard guard;

  const auto id = addTaskImpl(name, entry, pageDir);
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
      Pmm::freeFrame(reinterpret_cast<void *>(phys));
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

  const uint32_t phys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(phys));
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
    Pmm::freeFrame(reinterpret_cast<void *>(phys[i]));
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
      Pmm::freeFrame(reinterpret_cast<void *>(child.pageDir));
      child.pageDir = 0;
      return false;
    }

    const auto newPhys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(newPhys));
    const auto vaddr = parent.userStackVaddr[i];

    if (!Paging::mapPage(vaddr, newPhys32, PAGE_PRESENT | PAGE_RW | PAGE_USER, child.pageDir)) {
      Pmm::freeFrame(newPhys);
      freePageArray(i, child.userStackVaddr, child.userStackPhys);
      Pmm::freeFrame(reinterpret_cast<void *>(child.pageDir));
      child.pageDir = 0;
      return false;
    }

    const auto *src =
      static_cast<const uint8_t *>(physToVirt(reinterpret_cast<void *>(parent.userStackPhys[i])));
    auto *dst = static_cast<uint8_t *>(physToVirt(reinterpret_cast<void *>(newPhys32)));
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

  auto *codeDst = static_cast<uint8_t *>(physToVirt(reinterpret_cast<void *>(codeFrame.phys)));
  const size_t codeSize = static_cast<size_t>(userTestEnd - userTestStart);
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
  __builtin_memcpy(childStack + frameOffset, reinterpret_cast<const uint8_t *>(syscallFrameEsp),
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

  const auto *modPtr =
    physToVirt(reinterpret_cast<void *>(static_cast<uintptr_t>(Pmm::modulePhysStart(modIdx))));
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
    Pmm::freeFrame(reinterpret_cast<void *>(t.userCodeBuf));
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
  auto *frame = reinterpret_cast<uint32_t *>(syscallFrameEsp);

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
  InterruptGuard guard;

  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::exitCurrentTask: corrupted currentIdx");
  }

  if (tasks_[currentIdx_].onKill) {
    tasks_[currentIdx_].onKill();
  }

  tasks_[currentIdx_].exitCode = code;
  tasks_[currentIdx_].state = TaskState::DEAD;
  ++totalExited_;

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

void Scheduler::unblockTask(int id)
{
  if (id >= 0 && id < taskCount_ && tasks_[id].state == TaskState::BLOCKED) {
    tasks_[id].state = TaskState::READY;
    tasks_[id].wakeupMs = 0;
  }
}

int Scheduler::currentTaskId()
{
  return currentIdx_;
}

Task &Scheduler::currentTask()
{
  return tasks_[currentIdx_];
}

int Scheduler::aliveTaskCount()
{
  return countIf(isNotDead);
}

int Scheduler::runningReadyCount()
{
  return countIf(isRunningOrReady);
}

int Scheduler::blockedTaskCount()
{
  return countIf(isBlocked);
}

int Scheduler::deadTaskCount()
{
  return countIf(isDead);
}

int Scheduler::totalExited()
{
  return totalExited_;
}

int Scheduler::taskCount()
{
  return taskCount_;
}

optional<const char *> Scheduler::taskName(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].name.data();
}

optional<TaskState> Scheduler::taskState(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].state;
}

optional<uint8_t> Scheduler::taskFlags(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].flags;
}

optional<uint8_t> Scheduler::taskPriority(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].priority;
}

optional<uint32_t> Scheduler::taskEsp(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].esp;
}

optional<const uint8_t *> Scheduler::taskStackBuf(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].stackBuf;
}

optional<uint32_t> Scheduler::taskStackSize(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].stackSize;
}

optional<uint64_t> Scheduler::taskEntryAddr(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return reinterpret_cast<uint64_t>(tasks_[id].entry);
}

optional<uint64_t> Scheduler::taskWakeupMs(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].wakeupMs;
}

optional<uint64_t> Scheduler::taskRuntimeMs(int id)
{
  if (id < 0 || id >= taskCount_) {
    return {};
  }
  return tasks_[id].runtimeMs;
}

int Scheduler::quantumRemaining()
{
  return quantumRemaining_;
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
  const int unblockId = tasks_[id].unblockOnExit;
  tasks_[id].unblockOnExit = -1;
  if (unblockId != -1) {
    unblockTask(unblockId);
  }

  tasks_[id].state = TaskState::DEAD;
  ++totalExited_;

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
    Pmm::freeFrame(reinterpret_cast<void *>(tasks_[id].pageDir));
    tasks_[id].pageDir = 0;
  }

  // Free user-mode pages (ring-3 tasks).
  freePageArray(tasks_[id].userStackPageCount, tasks_[id].userStackVaddr, tasks_[id].userStackPhys);
  tasks_[id].userStackPageCount = 0;
  if (tasks_[id].userCodeBuf != 0) {
    Paging::unmapPage(USER_BASE);
    Pmm::freeFrame(reinterpret_cast<void *>(tasks_[id].userCodeBuf));
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

void Scheduler::sleep(uint64_t ms)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::sleep: corrupted currentIdx");
  }

  tasks_[currentIdx_].wakeupMs = Pit::uptimeMs() + ms;
  tasks_[currentIdx_].state = TaskState::BLOCKED;

  // Enable interrupts so `tick()` can be called, halt CPU until next interrupt, and then disable
  // interrupts because they are expected off and ISR will re-enable.
  while (tasks_[currentIdx_].state == TaskState::BLOCKED) {
#ifdef __IS_DOORS_KERNEL
    __asm__("sti\n\thlt\n\tcli");
#else
    break;
#endif
  }
}

void Scheduler::removeFromSleepQueue(int taskId)
{
  for (int i = 0; i < sleepCount_; ++i) {
    if (sleepQueue_[i].taskId == taskId) {
      --sleepCount_;

      // Shift remaining entries left to fill the gap and maintain sorted order.
      for (int j = i; j < sleepCount_; ++j) {
        sleepQueue_[j] = sleepQueue_[j + 1];
      }
      return;
    }
  }
}

void Scheduler::suppressTaskbar()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::suppressTaskbar: corrupted currentIdx");
  }
  tasks_[currentIdx_].flags |= Task::FLAG_SUPPRESS_TASKBAR;
}

void Scheduler::setOnKill(void (*handler)())
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::setOnKill: corrupted currentIdx");
  }
  tasks_[currentIdx_].onKill = handler;
}

bool Scheduler::isTaskbarSuppressed()
{
  return any_of(tasks_.begin(), tasks_.begin() + taskCount_, [](const Task &t) {
    return t.state != TaskState::DEAD && (t.flags & Task::FLAG_SUPPRESS_TASKBAR);
  });
}

void Scheduler::blockCurrentTask()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::blockCurrentTask: corrupted currentIdx");
  }
  tasks_[currentIdx_].state = TaskState::BLOCKED;
}

void Scheduler::blockCurrentTaskAndYield()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::blockCurrentTaskAndYield: corrupted currentIdx");
  }
  tasks_[currentIdx_].state = TaskState::BLOCKED;

#ifdef __IS_DOORS_KERNEL
  while (tasks_[currentIdx_].state == TaskState::BLOCKED) {
    __asm__("sti\n\thlt\n\tcli");
  }
#endif
}

void Scheduler::setUnblockOnExit(int taskId, int unblockId)
{
  if (taskId >= 0 && taskId < taskCount_) {
    tasks_[taskId].unblockOnExit = unblockId;
  }
}

optional<int> Scheduler::findNext()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::findNext: corrupted currentIdx");
  }
  if (taskCount_ < 0 || taskCount_ > MAX_TASKS) {
    panic("Scheduler::findNext: corrupted taskCount");
  }
  if (taskCount_ <= 1) {
    return {};
  }

  // Priority-based Round Robin selection.
  // Start with a value one worse than `PRIORITY_IDLE` so any `READY` task wins.
  int bestPriority = Task::PRIORITY_IDLE + 1;
  int bestIdx = -1;

  // Walk non-`idle` slots starting after the current task. Slots 1 through `taskCount_-1` contain
  // real tasks. Slot 0 is excluded so `idle` only runs when nothing better is found.
  for (int i = 0; i < taskCount_ - 1; ++i) {
    // +1 skips current task and modulo wraps and skips slot 0.
    const int idx = (currentIdx_ + 1 + i) % taskCount_;
    if (tasks_[idx].state == TaskState::READY && tasks_[idx].priority < bestPriority) {
      bestPriority = tasks_[idx].priority;
      bestIdx = idx;
    }

    // No task can beat `PRIORITY_HIGH`.
    if (bestPriority == Task::PRIORITY_HIGH) {
      break;
    }
  }
  return bestIdx >= 0 ? bestIdx : 0 /* idle */;
}

void Scheduler::checkCanary(const Task &t)
{
  if (t.stackBuf == nullptr) {
    return;
  }
  if (t.state == TaskState::DEAD) {
    return;
  }
  if (reinterpret_cast<const uint32_t *>(t.stackBuf)[0] != Task::STACK_CANARY) {
    panic("stack overflow");
  }
}
