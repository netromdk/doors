/**
 * These exception handlers will be called from isr.s if any are triggered.
 */

#ifndef KERNEL_I386_EXCEPTION_HANDLERS_H
#define KERNEL_I386_EXCEPTION_HANDLERS_H

#include <cstdint>
#include <cstdio>

#include <arch/i386/Paging.h>
#include <kernel/Cpu.h>
#include <kernel/Panic.h>
#include <kernel/Scheduler.h>

extern "C" {

void excDivZero()
{
  panic("division by zero");
}

void excInvOp(uint32_t *frame)
{
  // `frame` points to post-pushal stack from `asmExcInvOp`.
  // pushal layout: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX.
  // Above pushal (from CPU for ring3 -> 0 transition):
  //   frame[8]  = EIP
  //   frame[9]  = CS
  //   frame[10] = EFLAGS
  //   frame[11] = ESP_user
  //   frame[12] = SS_user
  const uint32_t eip = frame[8];
  const uint32_t cs = frame[9];
  printf("Invalid opcode! EIP=0x%x CS=0x%x\n", eip, cs);
  panic("invalid opcode");
}

void excSegNp()
{
  panic("segment not present");
}

void excSf()
{
  panic("stack fault");
}

void excGp()
{
  panic("general protection fault");
}

void excNm(uint32_t *)
{
  Scheduler::handleNm();
}

void excDf(uint32_t *frame)
{
  const auto faultAddr = Cpu::readCr2();
  printf("\nDOUBLE FAULT! faultAddr=0x%x frame=%p\n", faultAddr, frame);
  panic("double fault");
}

// Page fault handler.
void excPf(uint32_t *frame)
{
  // Read CR2 before anything that could fault. A nested page fault overwrites CR2 and triggers a
  // double fault -> triple fault -> silent reset.
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

  // CoW fault: present page (bit 0) + write access (bit 1). `&3 == 3` masks bits 0-1 and checks
  // both are set (3 = 0b11).
  if ((errCode & 3) == 3) {
    if (const auto pageDir = Scheduler::currentTask().pageDir;
        Paging::handleCowFault(faultAddr, pageDir)) {
      return;
    }
  }

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

  // If the fault came from userland, try to deliver `SIGSEGV` to the faulting task.
  if (cs & 3) {
    printf("  User ESP:      0x%x\n", frame[12]);
    printf("  User SS:       0x%x  (ring %d)\n", frame[13], frame[13] & 3);

    if (Scheduler::deliverSigsegvFromException(frame)) {
      return; // Signal handler installed and frame redirected.
    }

    printf("  Killing faulting userland task.\n");
    Scheduler::killFaultingTask(); // never returns.
  }

  panic("page fault");
}

} // extern "C"

#endif // KERNEL_I386_EXCEPTION_HANDLERS_H
