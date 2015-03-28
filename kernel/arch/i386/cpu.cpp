#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/cpu.h>

namespace {
  union cpu_features {
    uint32_t raw_data;
    struct {
      uint32_t FPU : 1;        // Floating-point Unit On-Chip
      uint32_t VME : 1;        // Virtual Mode Extension
      uint32_t DE : 1;         // Debugging Extension
      uint32_t PSE : 1;        // Page Size Extension
      uint32_t TSC : 1;        // Time Stamp Counter
      uint32_t MSR : 1;        // Model Specific Registers
      uint32_t PAE : 1;        // Physical Address Extension
      uint32_t MCE : 1;        // Machine-Check Exception
      uint32_t CX8 : 1;        // CMPXCHG8 Instruction
      uint32_t APIC : 1;       // On-chip APIC Hardware
      uint32_t RESERVED_1 : 1; 
      uint32_t SEP : 1;        // Fast System Call
      uint32_t MTRR : 1;       // Memory Type Range Registers
      uint32_t PGE : 1;        // Page Global Enable
      uint32_t MCA : 1;        // Machine-Check Architechture
      uint32_t CMOV : 1;       // Conditional Move Instruction
      uint32_t PAT : 1;        // Page Attribute Table
      uint32_t PSE36 : 1;      // Page Size Extension (36 bit)
      uint32_t PSN : 1;        // Processor serial number is present and enabled
      uint32_t CLFSH : 1;      // CLFLUSH Instruction
      uint32_t RESERVED_2 : 1; 
      uint32_t DS : 1;         // Debug Store
      uint32_t ACPI : 1;       // Thermal Monitor and Software Controlled Clock Facilities
      uint32_t MMX : 1;        // MMX Technology
      uint32_t FXSR : 1;       // FXSAVE and FXSTOR Instructions
      uint32_t SSE : 1;        // Streaming SIMD Extensions
      uint32_t SSE2 : 1;       // Streaming SIMD Extensions 2
      uint32_t SS : 1;         // Self-Snoop
      uint32_t HTT : 1;        // Multi-Threading
      uint32_t TM : 1;         // Thermal Monitor
      uint32_t RESERVED_3 : 1; 
      uint32_t PBE : 1;        // Pending Break Enable
    } features;
  };
  typedef union cpu_features cpu_features_t;

  // Extended features
  union cpu_efeatures {
    uint32_t raw_data;
    struct {
      uint32_t RESERVED_1 : 11;
      uint32_t SYSCALL : 1;     // Supports the SYSCALL and SYSRET instructions
      uint32_t RESERVED_2 : 8;
      uint32_t XDBit : 1;       // Execution Disable Bit (when PAE enabled)
      uint32_t RESERVED_3 : 8;
      uint32_t I64 : 1;         // Intel 64 Instruction Set Architecture
      uint32_t RESERVED_4 : 2;
    } features;
  };
  typedef union cpu_efeatures cpu_efeatures_t;

  // Extended features 2
  union cpu_efeatures2 {
    uint32_t raw_data;
    struct {
      uint32_t LAHF : 1;        // LAHF and SAHF instructions available when
                                // the IA-32e mode is enabled and the
                                // processor is operating in the 64-bit
                                // sub-mode.
      uint32_t RESERVED_1 : 31;
    } features;
  };
  typedef union cpu_efeatures2 cpu_efeatures2_t;
  
  bool cpuidSupported() {
    // This method uses the ID flag in bit 21 of the EFLAGS register. If
    // software can change the value of this flag, the CPUID instruction
    // is executable.

    uint32_t res;
    __asm__("mov %%ecx, %%eax;"
            "xor $200000, %%eax;"
            "push %%eax;"
            "pop %%eax;"
            "xor %%ecx, %%eax;"
            "je no;"
            "mov $1, %%eax;"
            "jmp end;"
            "no: mov $0, %%eax;"
            "end:;"
            : "=a" (res)
            :
            : "cc"); 
    return res == 1;
  }

  void cpuid(int code, uint32_t res[4]) {
    __asm__("cpuid"
            : "=a" (res[0]),
              "=b" (res[1]),
              "=c" (res[2]),
              "=d" (res[3])
            : "a" (code));
  }

  // Expects 'result' to have a length of 13.
  void cpuVendorId(uint8_t *result, uint32_t *func_max) {
    uint32_t res[4];

    // instruct cpuid to retrieve vendor id by using opcode 0
    cpuid(0, res);

    // first 4 bytes from EBX
    result[0] = res[1] & 255;
    result[1] = (res[1] >> 8) & 255;
    result[2] = (res[1] >> 16) & 255;
    result[3] = (res[1] >> 24) & 255;

    // next 4 bytes from EDX
    result[4] = res[3] & 255;
    result[5] = (res[3] >> 8) & 255;
    result[6] = (res[3] >> 16) & 255;
    result[7] = (res[3] >> 24) & 255;

    // last 4 bytes from ECX
    result[8] = res[2] & 255;
    result[9] = (res[2] >> 8) & 255;
    result[10] = (res[2] >> 16) & 255;
    result[11] = (res[2] >> 24) & 255;

    result[12] = 0;

    // as a side effect the maximum number of functions to query cpuid
    // with is provided.
    *func_max = res[0];  
  }

  // Expects 'result' to have a length of 49.
  void cpuBrandString(uint8_t *result) {
    uint32_t res[4];
    cpuid(0x80000002, res);

    result[0] = res[0] & 255;
    result[1] = (res[0] >> 8) & 255;
    result[2] = (res[0] >> 16) & 255;
    result[3] = (res[0] >> 24) & 255;

    result[4] = res[1] & 255;
    result[5] = (res[1] >> 8) & 255;
    result[6] = (res[1] >> 16) & 255;
    result[7] = (res[1] >> 24) & 255;

    result[8] = res[2] & 255;
    result[9] = (res[2] >> 8) & 255;
    result[10] = (res[2] >> 16) & 255;
    result[11] = (res[2] >> 24) & 255;

    result[12] = res[3] & 255;
    result[13] = (res[3] >> 8) & 255;
    result[14] = (res[3] >> 16) & 255;
    result[15] = (res[3] >> 24) & 255;

    cpuid(0x80000003, res);

    result[16] = res[0] & 255;
    result[17] = (res[0] >> 8) & 255;
    result[18] = (res[0] >> 16) & 255;
    result[19] = (res[0] >> 24) & 255;

    result[20] = res[1] & 255;
    result[21] = (res[1] >> 8) & 255;
    result[22] = (res[1] >> 16) & 255;
    result[23] = (res[1] >> 24) & 255;

    result[24] = res[2] & 255;
    result[25] = (res[2] >> 8) & 255;
    result[26] = (res[2] >> 16) & 255;
    result[27] = (res[2] >> 24) & 255;

    result[28] = res[3] & 255;
    result[29] = (res[3] >> 8) & 255;
    result[30] = (res[3] >> 16) & 255;
    result[31] = (res[3] >> 24) & 255;

    cpuid(0x80000004, res);

    result[32] = res[0] & 255;
    result[33] = (res[0] >> 8) & 255;
    result[34] = (res[0] >> 16) & 255;
    result[35] = (res[0] >> 24) & 255;

    result[36] = res[1] & 255;
    result[37] = (res[1] >> 8) & 255;
    result[38] = (res[1] >> 16) & 255;
    result[39] = (res[1] >> 24) & 255;

    result[40] = res[2] & 255;
    result[41] = (res[2] >> 8) & 255;
    result[42] = (res[2] >> 16) & 255;
    result[43] = (res[2] >> 24) & 255;

    result[44] = res[3] & 255;
    result[45] = (res[3] >> 8) & 255;
    result[46] = (res[3] >> 16) & 255;
    result[47] = (res[3] >> 24) & 255;    

    result[48] = 0;
  }
}

void dumpCpu() {
  if (!cpuidSupported()) {
    printf("cpuid instruction not supported cpu CPU!\n");
    return;
  }

  printf("CPU information:\n\n");

  // EAX=0: Get vendor id.
  uint32_t funcMax;
  uint8_t vendorId[13];
  cpuVendorId(vendorId, &funcMax);
  printf("  Vendor ID: %s\n", vendorId);

  // EAX=1: Get processor info and feature bits.
  uint32_t res[4];
  if (funcMax >= 1) {
    cpuid(1, res);
    int stepping = res[0] & 15,
      model = (res[0] >> 4) & 15,
      family = (res[0] >> 8) & 15,
      proc_type = (res[0] >> 12) & 15;
    printf("  Stepping: %d, Model: %d, Family: %d, Processor type: %d\n",
           stepping, model, family, proc_type);

    // Parse features.
    cpu_features_t features;
    features.raw_data = res[3];

    printf("  Features: ");
    if (features.features.FPU) printf("FPU, ");
    if (features.features.VME) printf("VME, ");
    if (features.features.DE) printf("DE, ");
    if (features.features.PSE) printf("PSE, ");
    if (features.features.TSC) printf("TSC, ");
    if (features.features.MSR) printf("MSR, ");
    if (features.features.PAE) printf("PAE, ");
    if (features.features.MCE) printf("MCE, ");
    if (features.features.CX8) printf("CX8, ");
    if (features.features.APIC) printf("APIC, ");
    if (features.features.SEP) printf("SEP, ");
    if (features.features.MTRR) printf("MTRR, ");
    if (features.features.PGE) printf("PGE, ");
    if (features.features.MCA) printf("MCA, ");
    if (features.features.CMOV) printf("CMOV, ");
    if (features.features.PAT) printf("PAT, ");
    if (features.features.PSE36) printf("PSE36, ");
    if (features.features.PSN) printf("PSN, ");
    if (features.features.CLFSH) printf("CLFSH, ");
    if (features.features.DS) printf("DS, ");
    if (features.features.ACPI) printf("ACPI, ");
    if (features.features.MMX) printf("MMX, ");
    if (features.features.FXSR) printf("FXSR, ");
    if (features.features.SSE) printf("SSE, ");
    if (features.features.SSE2) printf("SSE2, ");
    if (features.features.SS) printf("SS, ");
    if (features.features.HTT) printf("HTT, ");
    if (features.features.TM) printf("TM, ");
    if (features.features.PBE) printf("PBE, ");
    printf("\n");
  }

  // Determine which extended functions are available.
  cpuid(0x80000000, res);
  funcMax = res[0];

  // EAX=80000001h: Get extended processor info and feature bits.
  if (funcMax >= 0x80000001) {
    cpuid(0x80000001, res);

    // Parse extended features.
    cpu_efeatures_t efeatures;
    efeatures.raw_data = res[3];

    cpu_efeatures2_t efeatures2;
    efeatures2.raw_data = res[2];

    printf("  Extended features: ");
    if (efeatures.features.SYSCALL) printf("SYSCALL, ");
    if (efeatures.features.XDBit) printf("XDBit, ");
    if (efeatures.features.I64) printf("I64, ");
    if (efeatures2.features.LAHF) printf("LAHF, ");
    printf("\n");
  }

  // EAX=80000002h,80000003h,80000004h: Get processor brand string.
  if (funcMax >= 0x80000002 && funcMax >= 0x80000003 && funcMax >= 0x80000004) {
    uint8_t brand[49];
    cpuBrandString(brand);
    printf("  Brand string: %s\n", brand);
  }  
}