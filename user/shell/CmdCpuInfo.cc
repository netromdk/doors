#include "Commands.h"
#include "Lib.h"
#include "Util.h"
#include "lib/Syscall.h"

void cmdCpuInfo(const span<string_view> &)
{
  CpuInfoRaw cpu{};
  if (sys_sysinfo(SYSINFO_CPU, reinterpret_cast<unsigned int>(&cpu)) == 0) {
    print("CPU information:\n");
    print("  Vendor ID: ");
    for (int i = 0; i < 12; ++i) {
      putchar(cpu.vendor[i]);
    }
    putchar('\n');
    printf("  Stepping: %u, Model: %u, Family: %u, Processor type: %u\n",
           static_cast<unsigned>(cpu.stepping), static_cast<unsigned>(cpu.model),
           static_cast<unsigned>(cpu.family), static_cast<unsigned>(cpu.procType));
    print("  Features: ");
    if (cpu.features & (1 << 0)) print("FPU, ");
    if (cpu.features & (1 << 1)) print("VME, ");
    if (cpu.features & (1 << 2)) print("DE, ");
    if (cpu.features & (1 << 3)) print("PSE, ");
    if (cpu.features & (1 << 4)) print("TSC, ");
    if (cpu.features & (1 << 5)) print("MSR, ");
    if (cpu.features & (1 << 6)) print("PAE, ");
    if (cpu.features & (1 << 7)) print("MCE, ");
    if (cpu.features & (1 << 8)) print("CX8, ");
    if (cpu.features & (1 << 9)) print("APIC, ");
    if (cpu.features & (1 << 11)) print("SEP, ");
    if (cpu.features & (1 << 12)) print("MTRR, ");
    if (cpu.features & (1 << 13)) print("PGE, ");
    if (cpu.features & (1 << 14)) print("MCA, ");
    if (cpu.features & (1 << 15)) print("CMOV, ");
    if (cpu.features & (1 << 16)) print("PAT, ");
    if (cpu.features & (1 << 17)) print("PSE36, ");
    if (cpu.features & (1 << 18)) print("PSN, ");
    if (cpu.features & (1 << 19)) print("CLFSH, ");
    if (cpu.features & (1 << 21)) print("DS, ");
    if (cpu.features & (1 << 22)) print("ACPI, ");
    if (cpu.features & (1 << 23)) print("MMX, ");
    if (cpu.features & (1 << 24)) print("FXSR, ");
    if (cpu.features & (1 << 25)) print("SSE, ");
    if (cpu.features & (1 << 26)) print("SSE2, ");
    if (cpu.features & (1 << 27)) print("SS, ");
    if (cpu.features & (1 << 28)) print("HTT, ");
    if (cpu.features & (1 << 29)) print("TM, ");
    if (cpu.features & (1 << 31)) print("PBE, ");
    print("\n");
    print("  Extended features: ");
    if (cpu.extFeatures & (1 << 11)) print("SYSCALL, ");
    if (cpu.extFeatures & (1 << 20)) print("XDBit, ");
    if (cpu.extFeatures & (1 << 29)) print("I64, ");
    if (cpu.extFeaturesEcx & (1 << 0)) print("LAHF, ");
    print("\n");
    int bl = brandLen(cpu.brand);
    if (bl > 0) {
      print("  Brand string: ");
      for (int i = 0; i < bl; ++i) {
        putchar(cpu.brand[i]);
      }
      putchar('\n');
    }
    putchar('\n');
  }
}
