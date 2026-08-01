#ifndef SYS_VGA_H
#define SYS_VGA_H

#include <cstddef>

// VGA text-mode geometry shared between the kernel and userland.
constexpr size_t VGA_WIDTH = 80;
constexpr size_t VGA_HEIGHT = 25;

#endif
