#include <algorithm>
#include <kernel/Semaphore.h>

void Semaphore::wait()
{
  // Disable interrupts.
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  // Enable interrupts when `count_ > 0` and return.
  if (count_ > 0) {
    count_--;
#ifdef __IS_DOORS_KERNEL
    __asm__("sti");
#endif
    return;
  }

  // Scheduler not yet initialized. Early boot fallback and enable interrupts.
  if (Scheduler::taskCount() == 0) {
#ifdef __IS_DOORS_KERNEL
    __asm__("sti");
#endif
    return;
  }

  // All resources exhausted. Register as a waiter, then suspend until `signal()` moves it back to
  // READY. While suspended, busy-wait with `hlt` (the scheduler cannot switch to a BLOCKED task,
  // but the timer ISR still fires and `tick()` runs, which is how `signal()` from another task
  // eventually reaches `unblockTask()`).
  const int id = Scheduler::currentTaskId();
  waiters_[waitCount_++] = id;
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
#ifdef __IS_DOORS_KERNEL
  __asm__("cli");
#endif

  if (waitCount_ > 0) {
    // Wake the longest-waiting task. Take the first and shift the rest to one the left, such that
    // index 0 is removed.
    const int id = waiters_[0];
    const int count = waitCount_;
    copy(waiters_.data() + 1, waiters_.data() + count, waiters_.data());
    waitCount_--;
    Scheduler::unblockTask(id);
  }
  else {
    count_++;
  }

#ifdef __IS_DOORS_KERNEL
  __asm__("sti");
#endif
}
