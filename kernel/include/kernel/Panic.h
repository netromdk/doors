#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <cstdint>
#include <string_view>

struct CpuState {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
  uint32_t ss;
  uint32_t cr0;
  uint32_t cr2;
  uint32_t cr3;
};

void readCpuState(CpuState *state);
void dumpCpuState(const CpuState *state);
[[noreturn]] void panic(string_view msg);

#endif // KERNEL_PANIC_H
