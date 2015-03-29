#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/**
 * Detects information about the CPU and its supported features.
 */
bool initCpu();

/**
 * Writes detected information to term.
 */
void dumpCpu();

#endif // KERNEL_CPU_H
