#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H

// Syscall numbers for `int 0x80`.
enum Syscall {
  SYS_WRITE = 1,             // Write char to terminal (ebx = char).
  SYS_EXIT = 2,              // Exit current task.
  SYS_READ = 3,              // Read from keyboard (ebx=buf, ecx=count).
  SYS_WRITESTR = 4,          // Write string (ebx=buf, ecx=len).
  SYS_READLINE = 5,          // Read line with editing (ebx=buf, ecx=maxlen).
  SYS_IOCTL = 6,             // System control (ebx=cmd, ecx=arg).
  SYS_TASKCTL = 7,           // Task control (ebx=cmd, ecx=arg1, edx=arg2).
  SYS_SYSINFO = 8,           // System info (ebx=cmd, ecx=arg).
  SYS_EXECMOD = 9,           // Execute GRUB module (ebx=index).
  SYS_PANIC = 10,            // Kernel panic (ebx=msg).
  SYS_SUPPRESS_TASKBAR = 11, // Hide taskbar.
};

#endif // SYS_SYSCALL_H
