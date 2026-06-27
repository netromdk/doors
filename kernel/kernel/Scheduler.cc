#include <cstddef>
#include <cstdint>
#include <cstring>

#include <kernel/Heap.h>
#include <kernel/Panic.h>
#include <kernel/Scheduler.h>

Task Scheduler::tasks_[MAX_TASKS]{};
int Scheduler::taskCount_{0};
int Scheduler::currentIdx_{0};
int Scheduler::quantumRemaining_{0};
bool Scheduler::initialized_{false};

void Scheduler::init()
{
  for (int i = 0; i < MAX_TASKS; ++i) {
    tasks_[i] = {};
  }
  tasks_[0].state = TaskState::RUNNING;
  tasks_[0].id = 0;
  tasks_[0].entry = nullptr;
  tasks_[0].stackBuf = nullptr;
  tasks_[0].stackSize = 0;
  strncpy(tasks_[0].name, "shell", sizeof(tasks_[0].name) - 1);
  tasks_[0].name[sizeof(tasks_[0].name) - 1] = '\0';
  taskCount_ = 1;
  currentIdx_ = 0;
  quantumRemaining_ = QUANTUM_TICKS;
  initialized_ = true;
}

int Scheduler::addTask(const char *name, void (*entry)())
{
  // Disable interrupts while modifying the shared task table so the timer ISR, which calls
  // `tick()`, does not see a partially-initialized slot.
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  const int id = addTaskImpl(name, entry);

  // Re-enable interrupts after the new task slot is fully set up.
#ifdef __IS_DOORS_KERNEL
  __asm__("sti");
#endif

  return id;
}

int Scheduler::findSlot()
{
  for (int i = 0; i < taskCount_; ++i) {
    if (tasks_[i].state == TaskState::DEAD) {
      return i;
    }
  }
  if (taskCount_ >= MAX_TASKS) {
    return -1;
  }
  return taskCount_++;
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
  stackTop[-8] = 0;  // ESP_dummy – written by `pushal` but ignored by `popal`
  stackTop[-9] = 0;  // EBP
  stackTop[-10] = 0; // ESI
  stackTop[-11] = 0; // EDI

  return static_cast<uint32_t>(reinterpret_cast<unsigned long long>(stackTop - 11));
}

int Scheduler::addTaskImpl(const char *name, void (*entry)())
{
  const int slot = findSlot();
  if (slot < 0) {
    return -1;
  }

  Task &t = tasks_[slot];
  if (t.stackBuf != nullptr) {
    Heap::free(t.stackBuf);
    t.stackBuf = nullptr;
  }

  auto *stack = static_cast<uint8_t *>(Heap::alloc(TASK_STACK_SIZE));
  if (stack == nullptr) {
    return -1;
  }

  t.esp = initStackFrame(stack, &Scheduler::taskWrapper);
  t.entry = entry;
  t.state = TaskState::READY;
  t.id = static_cast<uint8_t>(slot);
  t.stackBuf = stack;
  t.stackSize = TASK_STACK_SIZE;
  strncpy(t.name, name, sizeof(t.name) - 1);
  t.name[sizeof(t.name) - 1] = '\0';

  return slot;
}

void Scheduler::taskWrapper()
{
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

  // Save the interrupted task's register frame pointer (`esp`).
  tasks_[currentIdx_].esp = currentEsp;

  // Detect stack overflow before it corrupts the saved frame.
  checkCanary(tasks_[currentIdx_]);

  // Charge one tick against the current task's quantum. If quantum remains, stay.
  if (--quantumRemaining_ > 0) {
    return 0;
  }

  // Quantum expired. Find the next READY task by round-robin approach.
  const int next = findNext();
  if (next < 0) {
    // No other runnable task exists. Keep running with a fresh quantum.
    quantumRemaining_ = QUANTUM_TICKS;
    return 0;
  }

  // Switch to the chosen task and return its saved `esp`.
  return switchTo(next);
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

int Scheduler::addTaskAndBlock(const char *name, void (*entry)())
{
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  const int id = addTaskImpl(name, entry);
  if (id < 0) {
#ifdef __IS_DOORS_KERNEL
    __asm__("sti");
#endif
    return -1;
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
  tasks_[currentIdx_].state = TaskState::DEAD;

  // Switch to the next READY task immediately instead of waiting for the next PIT tick to expire
  // the quantum, and thus eliminating up to ~20 ms of dead time.
  const int next = findNext();
  if (next >= 0) {
#ifdef __IS_DOORS_KERNEL
    const uint32_t esp = switchTo(next);

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
  }
}

int Scheduler::currentTaskId()
{
  return currentIdx_;
}

int Scheduler::findNext()
{
  if (currentIdx_ < 0 || currentIdx_ >= MAX_TASKS) {
    panic("Scheduler::findNext: corrupted currentIdx");
  }
  if (taskCount_ < 0 || taskCount_ > MAX_TASKS) {
    panic("Scheduler::findNext: corrupted taskCount");
  }
  if (taskCount_ <= 1) {
    return -1;
  }
  for (int i = 0; i < taskCount_ - 1; ++i) {
    const int idx = (currentIdx_ + 1 + i) % taskCount_;
    if (tasks_[idx].state == TaskState::READY) {
      return idx;
    }
  }
  return -1;
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
