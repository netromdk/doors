#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <kernel/Io.h>
#include <kernel/Bda.h>
#include <kernel/Acpi.h>

namespace {
  Rsd *rsdp = nullptr;
  Rsdt *rsdt = nullptr;
  Fadt *fadt = nullptr;

  Rsd *detectRsdp() {
    static const char *ID = "RSD PTR ";
    static const size_t idSize = strlen(ID);
  
    uint8_t *ptr = (uint8_t*) BDA_BASE_ADDR;
    bool found = false;
    for (size_t i = 0; i < 1024; i++, ptr++) {
      if (memcmp(ptr, ID, idSize) == 0) {
        found = true;
        break;
      }
    }

    if (!found) {
      uint8_t *endPtr = (uint8_t*) 0x000FFFFF;
      for (ptr = (uint8_t*) 0x000E0000; ptr != endPtr; ptr++) {
        if (memcmp(ptr, ID, idSize) == 0) {
          found = true;
          break;
        }
      }    
    }

    return (found ? (Rsd*) ptr : nullptr);
  }

  bool checkRsdp(Rsd *rsd) {
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(Rsd); i++) {
      sum += ((uint8_t*) rsd)[i];
    }
    return sum == 0;
  }

  bool checkSdt(Sdt *sdt) {
    uint8_t sum = 0;
    for (size_t i = 0; i < sdt->length; i++) {
      sum += ((uint8_t*) sdt)[i];
    }
    return sum == 0;
  }

  /**
   * Detect the FADT which has the signature "FACP".
   */
  Sdt *detectFadt(Rsdt *rsdt) {
    size_t nelm =
      (rsdt->header.length - sizeof(rsdt->header)) / sizeof(uint32_t);
    for (size_t i = 0; i < nelm; i++) {
      Sdt *sdt = (Sdt*) rsdt->otherSdts[i];
      if (strncmp(sdt->sig, "FACP", 4) == 0) {
        return sdt;
      }
    }
    return nullptr;
  }

  void cleanup() {
    rsdp = nullptr;
    rsdt = nullptr;
    fadt = nullptr;
  }
}

bool Acpi::init() {
  rsdp = detectRsdp();
  if (!rsdp) {
    printf("RSDP not found.\n");
    return false;
  }

  if (!checkRsdp(rsdp)) {
    cleanup();
    printf("RSDP checksum invalid.\n");
    return false;
  }

  // TODO: If revision is 1 = v2 then cast into version 2 structure.

  rsdt = (Rsdt*) rsdp->rsdtAddress;
  if (!checkSdt(&rsdt->header)) {
    cleanup();
    printf("RSDT checksum invalid.\n");
    return false;
  }

  fadt = (Fadt*) detectFadt(rsdt);
  if (!fadt) {
    cleanup();
    printf("FADT not found.\n");
    return false;
  }

  if (!checkSdt(&fadt->header)) {
    cleanup();
    printf("FADT checksum invalid.\n");
    return false;
  }

  if (fadt->smiCommandPort != 0 && fadt->acpiEnable != 0 &&
      fadt->acpiDisable != 0) {
    printf("Enabling ACPI explicitly.\n");
    Io::outb(fadt->smiCommandPort, fadt->acpiEnable);
  }

  return true;
}

bool Acpi::isSupported() {
  return rsdp && rsdt;
}
