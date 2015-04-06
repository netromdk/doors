#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H

class Mem {
public:
  /**
   * Detects the low memory (< 1M) boundary, or the bottom of the
   * Extended BIOS Data Area (BDA).
   */
  static uint16_t detectLowMem();
};

#endif // KERNEL_MEM_H
