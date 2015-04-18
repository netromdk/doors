#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" {
  int __cxa_guard_acquire(__guard *g);
  void __cxa_guard_release (__guard *g);
}

#endif // KERNEL_RUNTIME_H
