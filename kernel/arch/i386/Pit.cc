#include <algorithm>

#include <arch/i386/Pic.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Io.h>
#include <kernel/Pit.h>

volatile uint64_t pitTicks{};
static uint64_t pitDeadlineMs{};

void Pit::init()
{
  pitTicks = 0;
  pitDeadlineMs = 0;

  // Start with a 1 ms one-shot so the first tick advances `pitTicks` from 0 to 1. The scheduler
  // takes over tickless programming on the first `Scheduler::tick()`.
  programForMs(1);

  Pic::setMask(IRQ_TIMER, true);
}

void Pit::setDivisor(uint16_t divisor)
{
  Io::outb(PIT_CMD, PIT_CW_ONESHOT); // Select channel 0, lobyte/hibyte, mode 0 (one-shot).
  Io::outb(PIT_CH0, divisor & 0xFF); // Divisor low byte.
  Io::outb(PIT_CH0, divisor >> 8);   // Divisor high byte.
}

void Pit::programForMs(uint32_t ms)
{
  if (ms == 0) {
    ms = 1;
  }
  if (ms > PIT_MAX_MS) {
    ms = PIT_MAX_MS;
  }

  const InterruptGuard guard;
  auto ticks = static_cast<uint32_t>((PIT_BASE_CLOCK * ms) / 1000);
  ticks = clamp(ticks, uint32_t{1}, static_cast<uint32_t>(PIT_MAX_DIVISOR));

  pitDeadlineMs = pitTicks + ms;
  setDivisor(static_cast<uint16_t>(ticks));
}

void Pit::tick()
{
  // Advance wall clock to the deadline. ISR latency is a few microseconds, so this is a close
  // approximation of the actual current time.
  pitTicks = pitDeadlineMs;
}

uint64_t Pit::uptimeMs()
{
  const InterruptGuard guard;
  return pitTicks;
}

uint64_t Pit::uptimeSec()
{
  const InterruptGuard guard;
  return pitTicks / 1000;
}

uint64_t Pit::msSince(uint64_t last)
{
  const InterruptGuard guard;
  return pitTicks - last;
}

uint64_t Pit::deadline()
{
  const InterruptGuard guard;
  return pitDeadlineMs;
}
