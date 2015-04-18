#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <kernel/Bda.h>
#include <kernel/Acpi.h>

namespace {
  Rsd *rsdp = nullptr;
  
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

  bool checkRsdp(Rsd *ptr) {
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(Rsd); i++) {
      sum += ((uint8_t*) ptr)[i];
    }
    return sum == 0;
  }
}

bool Acpi::init() {
  rsdp = detectRsdp();
  if (!rsdp) {
    printf("RSDP not found.\n");
    return false;
  }

  if (!checkRsdp(rsdp)) {
    rsdp = nullptr;
    printf("RSDP checksum invalid.\n");
    return false;
  }

  printf("ACPI Version: %d\n", rsdp->revision + 1);
  // TODO: If revision is 1 = v2 then cast into version 2 structure.

  return true;
}

bool Acpi::isSupported() {
  return rsdp;
}
