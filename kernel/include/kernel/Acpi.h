#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H

#include <stdint.h>

// Root System Descriptor, version 1.
struct Rsd {
  uint8_t sig[8];
  uint8_t checksum;
  uint8_t oemId[6];
  uint8_t revision;
  uint32_t rsdtAddress;
} __attribute__ ((packed));

class Acpi {
public:
  static bool init();
  static bool isSupported();
};

#endif // KERNEL_ACPI_H
