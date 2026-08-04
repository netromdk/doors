#include "Commands.h"
#include "Lib.h"
#include "Util.h"
#include "lib/Syscall.h"

int cmdCpuInfo(const span<string_view> &)
{
  CpuInfoRaw cpu{};
  if (sys_sysinfo(SYSINFO_CPU, reinterpret_cast<unsigned int>(&cpu)) == 0) {
    printf("CPU information:\n");
    printf("  Vendor ID: ");
    for (int i = 0; i < 12; ++i) {
      putchar(cpu.vendor[i]);
    }
    putchar('\n');
    printf("  Stepping: %u, Model: %u, Family: %u, Processor type: %u\n",
           static_cast<unsigned>(cpu.stepping), static_cast<unsigned>(cpu.model),
           static_cast<unsigned>(cpu.family), static_cast<unsigned>(cpu.procType));
    printf("  Features: ");
    if (cpu.features & (1 << 0)) printf("FPU, ");
    if (cpu.features & (1 << 1)) printf("VME, ");
    if (cpu.features & (1 << 2)) printf("DE, ");
    if (cpu.features & (1 << 3)) printf("PSE, ");
    if (cpu.features & (1 << 4)) printf("TSC, ");
    if (cpu.features & (1 << 5)) printf("MSR, ");
    if (cpu.features & (1 << 6)) printf("PAE, ");
    if (cpu.features & (1 << 7)) printf("MCE, ");
    if (cpu.features & (1 << 8)) printf("CX8, ");
    if (cpu.features & (1 << 9)) printf("APIC, ");
    if (cpu.features & (1 << 11)) printf("SEP, ");
    if (cpu.features & (1 << 12)) printf("MTRR, ");
    if (cpu.features & (1 << 13)) printf("PGE, ");
    if (cpu.features & (1 << 14)) printf("MCA, ");
    if (cpu.features & (1 << 15)) printf("CMOV, ");
    if (cpu.features & (1 << 16)) printf("PAT, ");
    if (cpu.features & (1 << 17)) printf("PSE36, ");
    if (cpu.features & (1 << 18)) printf("PSN, ");
    if (cpu.features & (1 << 19)) printf("CLFSH, ");
    if (cpu.features & (1 << 21)) printf("DS, ");
    if (cpu.features & (1 << 22)) printf("ACPI, ");
    if (cpu.features & (1 << 23)) printf("MMX, ");
    if (cpu.features & (1 << 24)) printf("FXSR, ");
    if (cpu.features & (1 << 25)) printf("SSE, ");
    if (cpu.features & (1 << 26)) printf("SSE2, ");
    if (cpu.features & (1 << 27)) printf("SS, ");
    if (cpu.features & (1 << 28)) printf("HTT, ");
    if (cpu.features & (1 << 29)) printf("TM, ");
    if (cpu.features & (1 << 31)) printf("PBE, ");
    printf("\n");
    printf("  Extended features: ");
    if (cpu.extFeatures & (1 << 11)) printf("SYSCALL, ");
    if (cpu.extFeatures & (1 << 20)) printf("XDBit, ");
    if (cpu.extFeatures & (1 << 29)) printf("I64, ");
    if (cpu.extFeaturesEcx & (1 << 0)) printf("LAHF, ");
    printf("\n");
    int bl = brandLen(cpu.brand);
    if (bl > 0) {
      printf("  Brand string: ");
      for (int i = 0; i < bl; ++i) {
        putchar(cpu.brand[i]);
      }
      putchar('\n');
    }
    putchar('\n');
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
