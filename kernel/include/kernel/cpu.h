#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include <stdint.h>

#define CPUID_VENDOR_OLDAMD       "AMDisbetter!"
#define CPUID_VENDOR_AMD          "AuthenticAMD"
#define CPUID_VENDOR_INTEL        "GenuineIntel"
#define CPUID_VENDOR_VIA          "CentaurHauls"
#define CPUID_VENDOR_OLDTRANSMETA "TransmetaCPU"
#define CPUID_VENDOR_TRANSMETA    "GenuineTMx86"
#define CPUID_VENDOR_CYRIX        "CyrixInstead"
#define CPUID_VENDOR_CENTAUR      "CentaurHauls"
#define CPUID_VENDOR_NEXGEN       "NexGenDriven"
#define CPUID_VENDOR_UMC          "UMC UMC UMC "
#define CPUID_VENDOR_SIS          "SiS SiS SiS "
#define CPUID_VENDOR_NSC          "Geode by NSC"
#define CPUID_VENDOR_RISE         "RiseRiseRise"

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
  static bool hasVendorId(const char *id);

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
   * Read the contents of the EFLAGS register.
   */
  static uint32_t getEflags();
};

#endif // KERNEL_CPU_H
