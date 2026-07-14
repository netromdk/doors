#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

using __guard = int;

extern "C" {

int __cxa_guard_acquire(__guard *g);
void __cxa_guard_release(__guard *g);

}

#endif // KERNEL_RUNTIME_H
