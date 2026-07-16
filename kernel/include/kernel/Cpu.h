#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include <cstdint>
#include <string_view>
#include <kernel/Syscall.h>

static constexpr auto CPUID_VENDOR_OLDAMD = "AMDisbetter!",
  CPUID_VENDOR_AMD = "AuthenticAMD",
  CPUID_VENDOR_INTEL = "GenuineIntel",
  CPUID_VENDOR_VIA = "CentaurHauls",
  CPUID_VENDOR_OLDTRANSMETA = "TransmetaCPU",
  CPUID_VENDOR_TRANSMETA = "GenuineTMx86",
  CPUID_VENDOR_CYRIX = "CyrixInstead",
  CPUID_VENDOR_CENTAUR = "CentaurHauls",
  CPUID_VENDOR_NEXGEN = "NexGenDriven",
  CPUID_VENDOR_UMC = "UMC UMC UMC ",
  CPUID_VENDOR_SIS = "SiS SiS SiS ",
  CPUID_VENDOR_NSC = "Geode by NSC",
  CPUID_VENDOR_RISE = "RiseRiseRise";

class Cpu {
public:
  /**
   * Detects information about the CPU and its supported features.
   */
  static bool init();

  /**
   * Writes detected information to term.
   */
  static void dump();

  /**
   * Checks if the CPU has the specificed vendor ID.
   */
  static bool hasVendorId(string_view id);

  /**
   * Checks if the CPU supports Physical Address Extension (for 4+ GB
   * RAM).
   */
  static bool hasPae();

  /**
   * Checks if the CPU supports Time Stamp Counter.
   */
  static bool hasTsc();

  /**
   * Checks if the CPU supports SYSENTER/SYSEXIT routines.
   */
  static bool hasSysEnter();

  /**
   * Checks if the CPU has an on-chip FPU.
   */
  static bool hasFpu();

  /**
   * Checks if the CPU supports FXSAVE/FXRSTOR instructions.
   */
  static bool hasFxsr();

  /**
   * Reads the current time stamp.
   */
  static uint32_t getTimeStamp();

  /**
   * Determines the average amount of cycles for performing an FDIV.
   */
  static uint32_t getCyclesPrFDiv();

  /**
   * Read/write the EFLAGS register.
   */
  static uint32_t getEflags();
  static void setEflags(uint32_t eflags);

  /**
   * Read/query interrupt state.
   */
  static bool interruptsEnabled();

  /**
   * Disable/enable interrupts.
   */
  static void disableInterrupts();
  static void enableInterrupts();

  /**
   * Halt the processor until the next interrupt.
   */
  static void halt();

  /**
   * Triple fault: with QEMU `-no-reboot` this exits, and on real hardware it reboots.
   */
  __attribute__((noreturn)) static void tripleFault();

  /**
   * Read/write control registers for paging.
   */
  static uint32_t readCr0();
  static void writeCr0(uint32_t value);
  static uint32_t readCr2();
  static uint32_t readCr3();
  static void writeCr3(uint32_t value);

  /**
   * Flush a single 4 KiB page from the TLB.
   */
  static void invlpg(uint32_t virtAddr);

  /**
   * Read raw CPU info into user-provided output struct.
   */
  static void readCpuInfo(CpuInfoRaw &out);

  /**
   * FPU context management for lazy switching via CR0.TS.
   */
  static void fxsave(uint8_t *buf);
  static void fxrstor(const uint8_t *buf);
  static void fninit();
};

#endif // KERNEL_CPU_H
