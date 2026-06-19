#include <stdint.h>
#include <kernel/Vga.h>

static uint16_t vgaTestBuffer[80 * 25];
uint16_t *VGA_RAM = vgaTestBuffer;
