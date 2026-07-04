#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <cstdint>

enum Syscall : uint32_t {
  SYS_WRITE = 1, // Write a character to the terminal. EBX = char.
  SYS_EXIT = 2,  // Exit the current task.
  SYS_READ = 3,  // Read from keyboard into buffer. EBX=buf, ECX=count. Returns bytes read.
};

extern "C" uint32_t syscallHandler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif // KERNEL_SYSCALL_H
