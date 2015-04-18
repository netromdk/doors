#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H

#include <stdint.h>

// Root System Descriptor, version 1.
struct Rsd {
  char sig[8];
  uint8_t checksum;
  uint8_t oemId[6];
  uint8_t revision;
  uint32_t rsdtAddress;
} __attribute__ ((packed));

// System Descriptor Table
struct Sdt {
  char sig[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oemId[6];
  uint8_t oemTableID[8];
  uint32_t oemRevision;
  uint32_t creatorID;
  uint32_t creatorRevision;
} __attribute__ ((packed));

// Root System Descriptor Table that points to the other SDTs.
struct Rsdt {
  Sdt header;
  uint32_t *otherSdts;
} __attribute__ ((packed));

// Generic Address Structure.
struct Gas {
  uint8_t addressSpace;
  uint8_t bitWidth;
  uint8_t bitOffset;
  uint8_t accessSize;
  uint64_t address;
} __attribute__ ((packed));

// Fixed ACPI Description Table.
struct Fadt {
  Sdt header;

  uint32_t firmwareCtrl;
  uint32_t dsdt;
  uint8_t reserved;
  uint8_t rreferredPowerManagementProfile;
  uint16_t sciInterrupt;
  uint32_t smiCommandPort;
  uint8_t acpiEnable;
  uint8_t acpiDisable;
  uint8_t s4biosReq;
  uint8_t pstateControl;
  uint32_t pm1aEventBlock;
  uint32_t pm1bEventBlock;
  uint32_t pm1aControlBlock;
  uint32_t pm1bControlBlock;
  uint32_t pm2ControlBlock;
  uint32_t pmTimerBlock;
  uint32_t gpe0Block;
  uint32_t gpe1Block;
  uint8_t pm1EventLength;
  uint8_t pm1ControlLength;
  uint8_t pm2ControlLength;
  uint8_t pmTimerLength;
  uint8_t gpe0Length;
  uint8_t gpe1Length;
  uint8_t gpe1Base;
  uint8_t cStateControl;
  uint16_t worstC2Latency;
  uint16_t worstC3Latency;
  uint16_t flushSize;
  uint16_t flushStride;
  uint8_t dutyOffset;
  uint8_t dutyWidth;
  uint8_t dayAlarm;
  uint8_t monthAlarm;
  uint8_t century;

  // Reserved in ACPI 1.0 but ACPI 2.0 uses it.
  uint16_t bootArchitectureFlags;

  uint8_t reserved2;
  uint32_t flags;
  Gas resetReg;
  uint8_t resetValue;
  uint8_t reserved3[3];

  // 64-bit pointers - Available on ACPI 2.0+
  uint64_t xFirmwareControl;
  uint64_t xDsdt;

  Gas xPM1aEventBlock;
  Gas xPM1bEventBlock;
  Gas xPM1aControlBlock;
  Gas xPM1bControlBlock;
  Gas xPM2ControlBlock;
  Gas xPMTimerBlock;
  Gas xGPE0Block;
  Gas xGPE1Block;
} __attribute__ ((packed));

class Acpi {
public:
  static bool init();
  static bool isSupported();
};

#endif // KERNEL_ACPI_H
