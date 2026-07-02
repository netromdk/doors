#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include <cstdint>
#include <string_view>

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
};

#endif // KERNEL_CPU_H
