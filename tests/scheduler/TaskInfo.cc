#include <string.h>

#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

alignas(16) uint8_t testPool[262144];

} // namespace

TEST_CASE("taskCount: starts at 1 after init, increments with addTask")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  CHECK(Scheduler::taskCount() == 1); // Shell task.

  CHECK(*Scheduler::addTask("a", nullptr) == 1);
  CHECK(Scheduler::taskCount() == 2);

  CHECK(*Scheduler::addTask("b", nullptr) == 2);
  CHECK(Scheduler::taskCount() == 3);
}

TEST_CASE("taskName: returns stored name and empty string for bad id")
{
  Scheduler::init();

  CHECK(strcmp(*Scheduler::taskName(0), "shell") == 0);
  CHECK(strcmp(Scheduler::taskName(-1).value_or(""), "") == 0);
  CHECK(strcmp(Scheduler::taskName(99).value_or(""), "") == 0);
}

TEST_CASE("taskName: returns name of added task")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("foobar", nullptr);
  CHECK(strcmp(*Scheduler::taskName(1), "foobar") == 0);
}

TEST_CASE("taskState: returns RUNNING for task 0 after init")
{
  Scheduler::init();
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}

TEST_CASE("taskState: returns nullopt for invalid ids")
{
  Scheduler::init();
  CHECK(!Scheduler::taskState(-1));
  CHECK(!Scheduler::taskState(99));
}

TEST_CASE("taskState: added task starts READY, becomes DEAD after exit")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskState(1) == TaskState::READY);

  // Simulate exit by setting DEAD via test helper (test-only API).
  Scheduler::testSetTaskState(1, TaskState::DEAD);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE("taskFlags: returns 0 for added task, sets suppress correctly")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  CHECK(Scheduler::taskFlags(0) == 0);
  CHECK(!Scheduler::taskFlags(-1));
  CHECK(!Scheduler::taskFlags(99));

  // Suppress taskbar for the running task (shell in this case).
  Scheduler::suppressTaskbar();
  CHECK((*Scheduler::taskFlags(0) & Task::FLAG_SUPPRESS_TASKBAR) != 0);
}

TEST_CASE("taskEsp: returns saved esp for task 0")
{
  Scheduler::init();
  CHECK(Scheduler::taskEsp(0) == 0);
}

TEST_CASE("taskEsp: returns nullopt for invalid id")
{
  Scheduler::init();
  CHECK(!Scheduler::taskEsp(-1));
  CHECK(!Scheduler::taskEsp(99));
}

TEST_CASE("taskStackBuf: nullptr for shell, non-null for added task")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  CHECK(Scheduler::taskStackBuf(0) == nullptr);

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskStackBuf(1) != nullptr);
}

TEST_CASE("taskStackBuf: returns nullopt for invalid id")
{
  Scheduler::init();
  CHECK(!Scheduler::taskStackBuf(-1));
  CHECK(!Scheduler::taskStackBuf(99));
}

TEST_CASE("taskStackSize: 0 for shell, TASK_STACK_SIZE for added task")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  CHECK(Scheduler::taskStackSize(0) == 0);

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskStackSize(1) == Scheduler::TASK_STACK_SIZE);
}

// Only used by the next test case.
static void taskEntryTestDummy()
{
}

TEST_CASE("taskEntryAddr: 0 for shell, non-zero for added task")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  CHECK(Scheduler::taskEntryAddr(0) == 0);

  Scheduler::addTask("t", taskEntryTestDummy);
  CHECK(Scheduler::taskEntryAddr(1) != 0);
}

TEST_CASE("taskEntryAddr: returns nullopt for invalid id")
{
  Scheduler::init();
  CHECK(!Scheduler::taskEntryAddr(-1));
  CHECK(!Scheduler::taskEntryAddr(99));
}

TEST_CASE("taskWakeupMs: returns 0 initially")
{
  Scheduler::init();
  CHECK(Scheduler::taskWakeupMs(0) == 0);
}

TEST_CASE("taskWakeupMs: reflects sleep value")
{
  extern volatile uint64_t pitTicks;
  pitTicks = 5000;
  Scheduler::init();

  Scheduler::sleep(333);

  // In test mode, `sleep()` sets `wakeupMs = pitTicks + 333 = 5333`, then breaks the hlt loop
  // immediately so state stays BLOCKED.
  CHECK(Scheduler::taskWakeupMs(0) == 5333);
}

TEST_CASE("taskWakeupMs: cleared by tick after deadline")
{
  extern volatile uint64_t pitTicks;
  pitTicks = 2000;

  Scheduler::init();
  Scheduler::sleep(100);
  CHECK(Scheduler::taskWakeupMs(0) == 2100);

  pitTicks = 2200;
  Scheduler::tick(0);
  CHECK(Scheduler::taskWakeupMs(0) == 0);
}

TEST_CASE("taskWakeupMs: returns nullopt for invalid id")
{
  Scheduler::init();
  CHECK(!Scheduler::taskWakeupMs(-1));
  CHECK(!Scheduler::taskWakeupMs(99));
}

TEST_CASE("taskRuntimeMs: starts at 0")
{
  Scheduler::init();
  CHECK(Scheduler::taskRuntimeMs(0) == 0);
}

TEST_CASE("taskRuntimeMs: tick increments running task")
{
  Scheduler::init();
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 1);
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 2);
}

TEST_CASE("taskRuntimeMs: different tasks accumulate independently")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 1);

  Scheduler::addTask("t", nullptr);
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 2);
  CHECK(Scheduler::taskRuntimeMs(1) == 0);
}

TEST_CASE("taskRuntimeMs: returns nullopt for invalid id")
{
  Scheduler::init();
  CHECK(!Scheduler::taskRuntimeMs(-1));
  CHECK(!Scheduler::taskRuntimeMs(99));
}

TEST_CASE("quantumRemaining: starts at QUANTUM_TICKS")
{
  Scheduler::init();
  CHECK(Scheduler::quantumRemaining() == Scheduler::QUANTUM_TICKS);
}

TEST_CASE("quantumRemaining: decrements on tick")
{
  Scheduler::init();
  Scheduler::tick(0);
  CHECK(Scheduler::quantumRemaining() == Scheduler::QUANTUM_TICKS - 1);
}
