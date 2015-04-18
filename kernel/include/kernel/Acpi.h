#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H

class Acpi {
public:
  static bool init();
  static bool isSupported();
};

#endif // KERNEL_ACPI_H
