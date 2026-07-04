#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

// Must mirror `Syscall` in "include/kernel/Syscall.h"!
enum Syscall {
  SYS_WRITE = 1,
  SYS_EXIT = 2,
  SYS_READ = 3,
};

static inline void sys_write(char c)
{
  __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"((unsigned int) c) : "memory");
}

__attribute__((noreturn)) static inline void sys_exit()
{
  __asm__ volatile("int $0x80" : : "a"(SYS_EXIT));
  __builtin_unreachable();
}

#endif
