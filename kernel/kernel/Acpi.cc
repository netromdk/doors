#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <numeric>

#include <kernel/Acpi.h>
#include <kernel/Bda.h>
#include <kernel/Io.h>

namespace {
static constinit Rsd *rsdp = nullptr;
static constinit Rsdt *rsdt = nullptr;
static constinit Fadt *fadt = nullptr;
static constinit uint8_t centuryReg = 0;

Rsd *detectRsdp()
{
  static const char *ID = "RSD PTR ";
  static const size_t idSize = strlen(ID);

  uint8_t *ptr = (uint8_t *) BDA_BASE_ADDR;
  bool found = false;
  for (size_t i = 0; i < 1024; i++, ptr++) {
    if (memcmp(ptr, ID, idSize) == 0) {
      found = true;
      break;
    }
  }

  if (!found) {
    uint8_t *endPtr = (uint8_t *) 0x000FFFFF;
    for (ptr = (uint8_t *) 0x000E0000; ptr != endPtr; ptr++) {
      if (memcmp(ptr, ID, idSize) == 0) {
        found = true;
        break;
      }
    }
  }

  return (found ? (Rsd *) ptr : nullptr);
}

bool checkRsdp(Rsd *rsd)
{
  const auto *first = reinterpret_cast<uint8_t *>(rsd);
  return accumulate(first, first + sizeof(Rsd), uint8_t{0}) == 0;
}

bool checkSdt(Sdt *sdt)
{
  const auto *first = reinterpret_cast<uint8_t *>(sdt);
  return accumulate(first, first + sdt->length, uint8_t{0}) == 0;
}

/**
 * Detect the FADT which has the signature "FACP".
 *
 * The RSDT entry array follows the header in memory but may not be 4-byte aligned. A raw
 * `uint32_t*` implies natural alignment, and UBSan enforces this and aborts on misaligned
 * dereference. `PackedU32` is a wrapper that carries no alignment requirement, so the compiler
 * generates byte-at-a-time loads regardless of the runtime address.
 */
struct __attribute__((packed)) PackedU32 {
  uint32_t v;
};
Sdt *detectFadt(Rsdt *rsdt)
{
  const size_t nelm = (rsdt->header.length - sizeof(rsdt->header)) / sizeof(uint32_t);
  const auto *entries =
    reinterpret_cast<PackedU32 *>(reinterpret_cast<uintptr_t>(rsdt) + sizeof(rsdt->header));
  const auto it = find_if(entries, entries + nelm, [](const PackedU32 &entry) {
    const uint32_t ptr = entry.v;
    return strncmp(reinterpret_cast<Sdt *>(ptr)->sig, "FACP", 4) == 0;
  });
  return it != entries + nelm ? reinterpret_cast<Sdt *>(it->v) : nullptr;
}

void cleanup()
{
  rsdp = nullptr;
  rsdt = nullptr;
  fadt = nullptr;
}

} // namespace

bool Acpi::init()
{
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

  rsdt = (Rsdt *) rsdp->rsdtAddress;
  if (!checkSdt(&rsdt->header)) {
    cleanup();
    printf("RSDT checksum invalid.\n");
    return false;
  }

  fadt = (Fadt *) detectFadt(rsdt);
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

  // Cache the century register value before paging is enabled. The FADT pointer is a physical
  // address that may not be identity-mapped later.
  centuryReg = fadt->century;

  if (fadt->smiCommandPort != 0 && fadt->acpiEnable != 0 && fadt->acpiDisable != 0) {
    printf("+");
    Io::outb(fadt->smiCommandPort, fadt->acpiEnable);
  }

  return true;
}

bool Acpi::isSupported()
{
  return rsdp && rsdt && fadt;
}

Fadt *Acpi::getFadt()
{
  return fadt;
}

uint8_t Acpi::centuryRegister()
{
  return centuryReg;
}

void Acpi::disable()
{
  // Null out the stale physical pointers so they can never be dereferenced after paging is
  // enabled. The century register is cached during `init()` and remains available via
  // `centuryRegister()`.
  rsdp = nullptr;
  rsdt = nullptr;
  fadt = nullptr;
}
