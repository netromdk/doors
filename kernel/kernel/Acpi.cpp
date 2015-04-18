#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <kernel/Bda.h>
#include <kernel/Acpi.h>

namespace {
  // Root System Description Pointer.
  uint8_t *rsdp = nullptr;
  
  uint8_t *detectRsdp() {
    static const char *ID = "RSD PTR ";
    const size_t idSize = strlen(ID);
  
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

    return (found ? ptr : nullptr);
  }
}

bool Acpi::init() {
  rsdp = detectRsdp();
  if (!rsdp) return false;

  

  return true;
}

bool Acpi::isSupported() {
  return rsdp;
}
