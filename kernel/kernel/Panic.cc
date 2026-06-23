#include <arch/i386/Pic.h>
#include <cstdint>
#include <cstdio>
#include <kernel/Backtrace.h>
#include <kernel/Panic.h>
#include <kernel/Symbols.h>
#include <string>
#ifdef __IS_DOORS_KERNEL
#include <kernel/Tty.h>
#include <kernel/Vga.h>
#endif

void readCpuState(CpuState *state)
{
  if (!state) return;

  // This needed to be volatile to force the compiler to reload the pointer before every access. To
  // prevent register reuse across the ASM boundary (Clang was giving some issues..).
  volatile CpuState *sp = state;

#ifdef __x86_64__
  uint64_t tmp64[6];
  __asm__ __volatile__("movq %%rax, %0\n"
                       "movq %%rbx, %1\n"
                       "movq %%rcx, %2\n"
                       "movq %%rdx, %3\n"
                       "movq %%rsi, %4\n"
                       "movq %%rdi, %5\n"
                       : "=m"(tmp64[0]), "=m"(tmp64[1]), "=m"(tmp64[2]), "=m"(tmp64[3]),
                         "=m"(tmp64[4]), "=m"(tmp64[5]));
  sp->eax = static_cast<uint32_t>(tmp64[0]);
  sp->ebx = static_cast<uint32_t>(tmp64[1]);
  sp->ecx = static_cast<uint32_t>(tmp64[2]);
  sp->edx = static_cast<uint32_t>(tmp64[3]);
  sp->esi = static_cast<uint32_t>(tmp64[4]);
  sp->edi = static_cast<uint32_t>(tmp64[5]);

  uint64_t rbp_val, rsp_val;
  __asm__ __volatile__("movq %%rbp, %0\n"
                       "movq %%rsp, %1\n"
                       : "=m"(rbp_val), "=m"(rsp_val));
  sp->ebp = static_cast<uint32_t>(rbp_val);
  sp->esp = static_cast<uint32_t>(rsp_val);

  // Use `sp` as an "r" input: the ASM writes CS/DS/ES/FS/GS/SS directly into the struct at the
  // offsets shown. 16-bit `mov` writes the low 16 bits of each `uint32_t` field and the upper 16
  // bits stay 0 because the struct is zero-initialized.
  uint64_t efl_tmp;
  __asm__ __volatile__("mov %%cs, 0x24(%1)\n"
                       "mov %%ds, 0x2c(%1)\n"
                       "mov %%es, 0x30(%1)\n"
                       "mov %%fs, 0x34(%1)\n"
                       "mov %%gs, 0x38(%1)\n"
                       "mov %%ss, 0x3c(%1)\n"
                       "pushfq\n"
                       "popq %0\n"
                       : "=m"(efl_tmp)
                       : "r"(sp)
                       : "memory");
  sp->eflags = static_cast<uint32_t>(efl_tmp);
#else  // !__x86_64__
  uint32_t tmp[6];
  __asm__ __volatile__("movl %%eax, %0\n"
                       "movl %%ebx, %1\n"
                       "movl %%ecx, %2\n"
                       "movl %%edx, %3\n"
                       "movl %%esi, %4\n"
                       "movl %%edi, %5\n"
                       : "=m"(tmp[0]), "=m"(tmp[1]), "=m"(tmp[2]), "=m"(tmp[3]), "=m"(tmp[4]),
                         "=m"(tmp[5]));
  sp->eax = tmp[0];
  sp->ebx = tmp[1];
  sp->ecx = tmp[2];
  sp->edx = tmp[3];
  sp->esi = tmp[4];
  sp->edi = tmp[5];

  uint32_t ds_v, es_v, fs_v, gs_v, ss_v;
  __asm__ __volatile__("movl %%ebp, %0\n"
                       "movl %%esp, %1\n"
                       "mov %%cs, %2\n"
                       "pushf\n"
                       "pop %3\n"
                       "mov %%ds, %4\n"
                       "mov %%es, %5\n"
                       "mov %%fs, %6\n"
                       "mov %%gs, %7\n"
                       "mov %%ss, %8\n"
                       : "=m"(sp->ebp), "=m"(sp->esp), "=m"(sp->cs), "=m"(sp->eflags), "=r"(ds_v),
                         "=r"(es_v), "=r"(fs_v), "=r"(gs_v), "=r"(ss_v));
  sp->ds = ds_v;
  sp->es = es_v;
  sp->fs = fs_v;
  sp->gs = gs_v;
  sp->ss = ss_v;
#endif // !__x86_64__

#ifdef __IS_DOORS_KERNEL
  // CPU control registers require ring 0, and would otherwise raise a General Protection Fault.
  uint32_t cr0_v, cr2_v, cr3_v;
  __asm__ __volatile__("mov %%cr0, %0\n"
                       "mov %%cr2, %1\n"
                       "mov %%cr3, %2\n"
                       : "=r"(cr0_v), "=r"(cr2_v), "=r"(cr3_v));
  sp->cr0 = cr0_v;
  sp->cr2 = cr2_v;
  sp->cr3 = cr3_v;
#else
  sp->cr0 = 0;
  sp->cr2 = 0;
  sp->cr3 = 0;
#endif
}

void dumpCpuState(const CpuState *state)
{
  if (!state) return;

  printf("CS=%x  EIP=%x  EFLAGS=%x\n", state->cs, state->eip, state->eflags);
  printf("EAX=%x  EBX=%x  ECX=%x  EDX=%x\n", state->eax, state->ebx, state->ecx, state->edx);
  printf("ESI=%x  EDI=%x  EBP=%x  ESP=%x\n", state->esi, state->edi, state->ebp, state->esp);
  printf("DS=%x  ES=%x  FS=%x  GS=%x  SS=%x", state->ds, state->es, state->fs, state->gs,
         state->ss);
#ifdef __IS_DOORS_KERNEL
  printf("  CR0=%x  CR2=%x  CR3=%x", state->cr0, state->cr2, state->cr3);
#endif
  printf("\n");
}

[[noreturn]] void panic(const char *msg)
{
  Pic::disableInt();

  printf("\n\nKernel: I'm sorry Dave, I'm afraid I can't do that\n");
  printf("Reason: %s\n\n", msg);

  CpuState state{};
  readCpuState(&state);
  state.eip =
    static_cast<uint32_t>(reinterpret_cast<unsigned long long>(__builtin_return_address(0)));
  dumpCpuState(&state);

  printf("\n");
  dumpBacktrace();

#ifdef __IS_DOORS_KERNEL
  Tty::setColor(vgaColor(COLOR_WHITE, COLOR_RED));
  Tty::putLine(" KERNEL PANIC!", 0);
#endif

  for (;;) {
    asm volatile("hlt");
  }
}
