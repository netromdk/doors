#ifndef KERNEL_INTERRUPT_GUARD_H
#define KERNEL_INTERRUPT_GUARD_H

#include <kernel/Cpu.h>

class InterruptGuard {
public:
  InterruptGuard() : savedFlags_(Cpu::getEflags())
  {
    Cpu::disableInterrupts();
  }

  ~InterruptGuard()
  {
    Cpu::setEflags(savedFlags_);
  }

  InterruptGuard(const InterruptGuard &) = delete;
  InterruptGuard &operator=(const InterruptGuard &) = delete;
  InterruptGuard(InterruptGuard &&) = delete;
  InterruptGuard &operator=(InterruptGuard &&) = delete;

private:
  uint32_t savedFlags_;
};

#endif // KERNEL_INTERRUPT_GUARD_H
