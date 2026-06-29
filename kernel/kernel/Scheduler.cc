#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <volatile.h>

#include <kernel/Heap.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>

array<Task, Scheduler::MAX_TASKS> Scheduler::tasks_{};
volatile int Scheduler::taskCount_{0};
volatile int Scheduler::currentIdx_{0};
volatile int Scheduler::quantumRemaining_{0};
volatile bool Scheduler::initialized_{false};
int Scheduler::totalExited_{0};

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
  tasks_[0].entry = nullptr;
  tasks_[0].stackBuf = nullptr;
  tasks_[0].stackSize = 0;
  strncpy(tasks_[0].name.data(), "shell", tasks_[0].name.size() - 1);
  tasks_[0].name[tasks_[0].name.size() - 1] = '\0';
  taskCount_ = 1;
  currentIdx_ = 0;
  quantumRemaining_ = QUANTUM_TICKS;
  initialized_ = true;
}

optional<int> Scheduler::addTask(string_view name, void (*entry)())
{
  // Disable interrupts while modifying the shared task table so the timer ISR, which calls
  // `tick()`, does not see a partially-initialized slot.
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  const auto id = addTaskImpl(name, entry);

  // Re-enable interrupts after the new task slot is fully set up.
#ifdef __IS_DOORS_KERNEL
  __asm__("sti");
#endif

  return id;
}

optional<int> Scheduler::findSlot()
{
  const auto it = find_if(tasks_.begin(), tasks_.begin() + taskCount_,
                          [](const Task &t) { return t.state == TaskState::DEAD; });
  if (it != tasks_.begin() + taskCount_) {
    return static_cast<int>(it - tasks_.begin());
  }
  if (taskCount_ >= MAX_TASKS) {
    return nullopt;
  }
  const int n = volatileLoad(taskCount_);
  volatileStore(taskCount_, n + 1);
  return n;
}

uint32_t Scheduler::initStackFrame(uint8_t *stack, void (*entry)())
{
  // Canary at the base of the buffer to detect overflow.
  reinterpret_cast<uint32_t *>(stack)[0] = Task::STACK_CANARY;

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

optional<int> Scheduler::addTaskImpl(string_view name, void (*entry)())
{
  if (name.data() == nullptr) {
    return nullopt;
  }

  const auto slotOpt = findSlot();
  if (!slotOpt) {
    return nullopt;
  }
  const int slot = *slotOpt;

  Task &t = tasks_[slot];
  if (t.stackBuf != nullptr) {
    Heap::free(t.stackBuf);
    t.stackBuf = nullptr;
  }
  t.flags = 0;
  t.wakeupMs = 0;
  t.runtimeMs = 0;
  t.onKill = nullptr;

  auto *stack = static_cast<uint8_t *>(Heap::alloc(TASK_STACK_SIZE));
  if (stack == nullptr) {
    return nullopt;
  }

  t.esp = initStackFrame(stack, &Scheduler::taskWrapper);
  t.entry = entry;
  t.state = TaskState::READY;
  t.id = static_cast<uint8_t>(slot);
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
  exitCurrentTask();
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
  return tasks_[currentIdx_].esp;
}

optional<int> Scheduler::addTaskAndBlock(string_view name, void (*entry)())
{
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  const auto id = addTaskImpl(name, entry);
  if (!id) {
#ifdef __IS_DOORS_KERNEL
    __asm__("sti");
#endif
    return nullopt;
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

#ifdef __IS_DOORS_KERNEL
  __asm__("sti");
#endif
  return id;
}

[[noreturn]] void Scheduler::exitCurrentTask()
{
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::exitCurrentTask: corrupted currentIdx");
  }

  if (tasks_[currentIdx_].onKill) {
    tasks_[currentIdx_].onKill();
  }

  tasks_[currentIdx_].state = TaskState::DEAD;
  ++totalExited_;

  // Switch to the next READY task immediately instead of waiting for the next PIT tick to expire
  // the quantum, and thus eliminating up to ~20 ms of dead time.
  const auto next = findNext();
  if (next) {
#ifdef __IS_DOORS_KERNEL
    const uint32_t esp = switchTo(*next);

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
    return nullopt;
  }
  return tasks_[id].name.data();
}

optional<TaskState> Scheduler::taskState(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].state;
}

optional<uint8_t> Scheduler::taskFlags(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].flags;
}

optional<uint32_t> Scheduler::taskEsp(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].esp;
}

optional<const uint8_t *> Scheduler::taskStackBuf(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].stackBuf;
}

optional<uint32_t> Scheduler::taskStackSize(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].stackSize;
}

optional<uint64_t> Scheduler::taskEntryAddr(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return reinterpret_cast<uint64_t>(tasks_[id].entry);
}

optional<uint64_t> Scheduler::taskWakeupMs(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
  }
  return tasks_[id].wakeupMs;
}

optional<uint64_t> Scheduler::taskRuntimeMs(int id)
{
  if (id < 0 || id >= taskCount_) {
    return nullopt;
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

  tasks_[id].state = TaskState::DEAD;
  ++totalExited_;

  // Free memory used by task.
  if (tasks_[id].stackBuf != nullptr) {
    Heap::free(tasks_[id].stackBuf);
    tasks_[id].stackBuf = nullptr;
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

optional<int> Scheduler::findNext()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::findNext: corrupted currentIdx");
  }
  if (taskCount_ < 0 || taskCount_ > MAX_TASKS) {
    panic("Scheduler::findNext: corrupted taskCount");
  }
  if (taskCount_ <= 1) {
    return nullopt;
  }
  for (int i = 0; i < taskCount_ - 1; ++i) {
    const int idx = (currentIdx_ + 1 + i) % taskCount_;
    if (tasks_[idx].state == TaskState::READY) {
      return idx;
    }
  }
  return nullopt;
}

#ifndef __IS_DOORS_KERNEL
void Scheduler::testSetTaskState(int id, TaskState s)
{
  if (id >= 0 && id < MAX_TASKS) {
    tasks_[id].state = s;
  }
}

const Task *Scheduler::testGetTask(int id)
{
  if (id >= 0 && id < MAX_TASKS) {
    return &tasks_[id];
  }
  return nullptr;
}

void Scheduler::testSetCurrentIdx(int id)
{
  if (id >= 0 && id < MAX_TASKS) {
    currentIdx_ = id;
  }
}

void Scheduler::testSetTaskFlags(int id, uint8_t f)
{
  if (id >= 0 && id < MAX_TASKS) {
    tasks_[id].flags = f;
  }
}

void Scheduler::testSetTaskWakeupMs(int id, uint64_t ms)
{
  if (id >= 0 && id < MAX_TASKS) {
    tasks_[id].wakeupMs = ms;
  }
}

void Scheduler::testSetTaskRuntimeMs(int id, uint64_t ms)
{
  if (id >= 0 && id < MAX_TASKS) {
    tasks_[id].runtimeMs = ms;
  }
}

void Scheduler::testSetOnKill(int id, void (*handler)())
{
  if (id >= 0 && id < MAX_TASKS) {
    tasks_[id].onKill = handler;
  }
}
#endif

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
