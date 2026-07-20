#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <volatile.h>

#include <kernel/Heap.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>

#ifdef __IS_DOORS_KERNEL
#include <arch/i386/Gdt.h>
#include <arch/i386/Paging.h>
#endif

array<Task, Scheduler::MAX_TASKS> Scheduler::tasks_{};
volatile int Scheduler::taskCount_{0};
volatile int Scheduler::currentIdx_{0};
uint64_t Scheduler::quantumStartMs_{0};
volatile bool Scheduler::initialized_{false};
int Scheduler::totalExited_{0};
uint8_t Scheduler::nextPid_{0};
int Scheduler::fpuOwner_{-1};
array<Scheduler::SleepEntry, Scheduler::MAX_SLEEPERS> Scheduler::sleepQueue_{};
int Scheduler::sleepCount_{0};
uint64_t Scheduler::lastTickMs_{0};

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
  quantumStartMs_ = 0;
  sleepCount_ = 0;
  lastTickMs_ = 0;
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

  // Deliver pending signals to userland tasks. This modifies the saved frame so the interrupt
  // return will redirect to the signal handler.
  deliverPendingSignals();

  // Charge elapsed CPU time to the running task.
  const auto now = Pit::uptimeMs();
  const auto elapsed = now - lastTickMs_;
  lastTickMs_ = now;
  tasks_[currentIdx_].runtimeMs += elapsed;

  // Wake up BLOCKED tasks whose sleep deadline has passed.
  while (sleepCount_ > 0 && sleepQueue_[0].deadline <= now) {
    // Pop the head entry and shift remaining entries left to maintain sorted order.
    const auto entry = sleepQueue_[0];
    --sleepCount_;
    for (int i = 0; i < sleepCount_; ++i) {
      sleepQueue_[i] = sleepQueue_[i + 1];
    }

    // Guard: the task may have been woken by `unblockTask()` before the timer fired.
    if (tasks_[entry.taskId].state == TaskState::BLOCKED) {
      tasks_[entry.taskId].state = TaskState::READY;
    }
    tasks_[entry.taskId].wakeupMs = 0;
  }

  // Check quantum expiry using wall-clock time. Any task (including idle) that exhausts its quantum
  // yields to `findNext()`. If `findNext()` returns the current task itself, due to no better
  // candidate, reset the quantum and keep running.
  const auto quantumElapsed = now - quantumStartMs_;
  if (quantumElapsed >= QUANTUM_MS) {
    const auto next = findNext();
    if (!next || *next == currentIdx_) {
      // No other runnable task exists. Reset quantum and keep running.
      quantumStartMs_ = now;
      programNextTick();
      return 0;
    }

    // Switch to the chosen task and return its saved `esp`.
    return switchTo(*next);
  }

  // Program PIT for the next event.
  programNextTick();
  return 0;
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
  quantumStartMs_ = Pit::uptimeMs();

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

  programNextTick();
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

void Scheduler::unblockTask(int id)
{
  if (id >= 0 && id < taskCount_ && tasks_[id].state == TaskState::BLOCKED) {
    tasks_[id].state = TaskState::READY;
    tasks_[id].wakeupMs = 0;
    removeFromSleepQueue(id);

    // If the unblocked task has higher priority than the current task, reprogram the PIT to fire
    // soon so the scheduler can preempt the current task.
    if (tasks_[id].priority < tasks_[currentIdx_].priority) {
      Pit::programForMs(1);
    }
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
  const auto now = Pit::uptimeMs();
  const auto elapsed = now - quantumStartMs_;
  if (elapsed >= QUANTUM_MS) {
    return 0;
  }
  return static_cast<int>(QUANTUM_MS - elapsed);
}

void Scheduler::sleep(uint64_t ms)
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::sleep: corrupted currentIdx");
  }

  const auto deadline = Pit::uptimeMs() + ms;
  tasks_[currentIdx_].wakeupMs = deadline;
  tasks_[currentIdx_].state = TaskState::BLOCKED;

  // Insert into sorted sleep queue with ascending deadline ordering. Walk backwards from the end to
  // find the correct position, shifting larger deadlines right.
  int pos = sleepCount_;
  while (pos > 0 && sleepQueue_[pos - 1].deadline > deadline) {
    sleepQueue_[pos] = sleepQueue_[pos - 1];
    --pos;
  }
  sleepQueue_[pos] = {deadline, currentIdx_};
  ++sleepCount_;

  // If the new deadline is sooner than the current PIT deadline, reprogram the PIT to fire at the
  // new deadline so the task wakes up on time.
  if (deadline < Pit::deadline()) {
    Pit::programForMs(static_cast<uint32_t>(ms));
  }

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

void Scheduler::programNextTick()
{
  const auto now = Pit::uptimeMs();
  uint32_t nextMs = PIT_MAX_MS;

  // Use the earliest sleep deadline if one exists.
  if (sleepCount_ > 0) {
    if (const auto sleepDeadline = sleepQueue_[0].deadline; sleepDeadline > now) {
      nextMs = static_cast<uint32_t>(sleepDeadline - now);
    }
    else {
      nextMs = 1;
    }
  }

  // Consider the quantum expiry for all tasks (including idle) so the PIT fires in time to preempt
  // idle when a higher-priority READY task is waiting.
  if (const auto elapsed = now - quantumStartMs_; elapsed < QUANTUM_MS) {
    if (const auto remaining = QUANTUM_MS - elapsed; remaining < nextMs) {
      nextMs = static_cast<uint32_t>(remaining);
    }
  }

  nextMs = clamp(nextMs, uint32_t{1}, PIT_MAX_MS);
  Pit::programForMs(nextMs);
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
    tasks_[taskId].unblockOnExit = static_cast<int8_t>(unblockId);
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
