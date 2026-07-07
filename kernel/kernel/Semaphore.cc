#include <algorithm>
#include <utility>
#include <volatile.h>

#include <kernel/InterruptGuard.h>
#include <kernel/Panic.h>
#include <kernel/Semaphore.h>

void Semaphore::wait()
{
  InterruptGuard guard;

  // Non-zero count: decrement and return.
  if (volatileLoad(count_) > 0) {
    volatileStore(count_, volatileLoad(count_) - 1);
    return;
  }

  // Scheduler not yet initialized: early boot fallback.
  if (Scheduler::taskCount() == 0) {
    return;
  }

  // All resources exhausted. Register as a waiter, then suspend until `signal()` moves it back to
  // READY. While suspended, busy-wait with `hlt` (the scheduler cannot switch to a BLOCKED task,
  // but the timer ISR still fires and `tick()` runs, which is how `signal()` from another task
  // eventually reaches `unblockTask()`).
  const int id = Scheduler::currentTaskId();
  const int wc = volatileLoad(waitCount_);
  if (wc >= MAX_WAITERS) {
#ifdef __IS_DOORS_KERNEL
    panic("Semaphore::wait: waiter count exceeds MAX_WAITERS");
#else
    return;
#endif
  }
  volatileStore(waitCount_, wc + 1);
  waiters_[wc] = id;
  Scheduler::blockCurrentTask();

#ifdef __IS_DOORS_KERNEL
  while (Scheduler::taskState(id) == TaskState::BLOCKED) {
    __asm__("sti\n\thlt\n\tcli");
  }
#else
  // Test mode: break so the test can inspect state and drive `signal()` manually.
#endif
}

void Semaphore::signal()
{
  InterruptGuard guard;

  if (waitCount_ > 0) {
    // Wake the longest-waiting task. Take the first and shift the rest to one the left, such that
    // index 0 is removed.
    const int id = waiters_[0];
    const int count = exchange(waitCount_, waitCount_ - 1);
    copy(waiters_.data() + 1, waiters_.data() + count, waiters_.data());
    Scheduler::unblockTask(id);
  }
  else {
    volatileStore(count_, volatileLoad(count_) + 1);
  }
}
