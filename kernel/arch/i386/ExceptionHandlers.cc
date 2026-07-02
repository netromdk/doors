/**
 * These exception handlers will be called from isr.s if any are triggered.
 */

#ifndef KERNEL_I386_EXCEPTION_HANDLERS_H
#define KERNEL_I386_EXCEPTION_HANDLERS_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <kernel/Cpu.h>
#include <kernel/Panic.h>

extern "C" {

void excDivZero()
{
  printf("Divide by zero!\n");
  abort();
}

void excInvOp()
{
  printf("Invalid opcode!\n");
  abort();
}

void excSegNp()
{
  printf("Segment not present!\n");
  abort();
}

void excSf()
{
  printf("Stack fault!\n");
  abort();
}

void excGp()
{
  printf("General protection exception!\n");
  abort();
}

// Page fault handler.
void excPf(uint32_t *frame)
{
  // CR2 contains the linear address that caused the fault.
  const auto faultAddr = Cpu::readCr2();

  // `frame` points to the post-pushal stack passed by `asmExcPf` in Isr.s.
  // After pushal, the stack frame is:
  //   frame[0]  = EDI          (pushal, saved last / lowest address)
  //   frame[1]  = ESI
  //   frame[2]  = EBP
  //   frame[3]  = original ESP (before pushal)
  //   frame[4]  = EBX
  //   frame[5]  = EDX
  //   frame[6]  = ECX
  //   frame[7]  = EAX          (pushal, saved first / highest address)
  //   frame[8]  = error code   (CPU-pushed for #PF)
  //   frame[9]  = EIP          (faulting instruction address)
  //   frame[10] = CS           (code segment / CPL at time of fault)
  //   frame[11] = EFLAGS
  //   frame[12] = ESP_user     (only if ring 3 -> ring 0 privilege change)
  //   frame[13] = SS_user      (only if ring 3 -> ring 0)
  const auto errCode = frame[8];
  const auto eip = frame[9];
  const auto cs = frame[10];
  const auto eflags = frame[11];

  printf("\nPage fault!\n");
  printf("  Fault address: 0x%x\n", faultAddr);
  printf("  EIP:           0x%x\n", eip);
  printf("  CS:            0x%x  (ring %d)\n", cs, cs & 3);
  printf("  EFLAGS:        0x%x\n", eflags);
  printf("  Error code:    0x%x  [%s%s%s%s%s]\n", errCode,
         (errCode & (1 << 0)) ? "protection " : "non-present ",
         (errCode & (1 << 1)) ? "write " : "read ", (errCode & (1 << 2)) ? "user " : "supervisor ",
         (errCode & (1 << 3)) ? "reserved-bit " : "",
         (errCode & (1 << 4)) ? "instruction-fetch " : "");

  // If the fault came from user mode, dump the user stack pointer too.
  if (cs & 3) {
    printf("  User ESP:      0x%x\n", frame[12]);
    printf("  User SS:       0x%x  (ring %d)\n", frame[13], frame[13] & 3);
  }

  panic("page fault");
}

} // extern "C"

#endif // KERNEL_I386_EXCEPTION_HANDLERS_H
