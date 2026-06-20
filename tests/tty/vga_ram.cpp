#include <stdint.h>
#include <kernel/Vga.h>

static uint16_t vgaTestBuffer[VGA_WIDTH * VGA_HEIGHT];
uint16_t *VGA_RAM = vgaTestBuffer;
