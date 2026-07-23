#ifndef TESTS_PAGING_PMMTESTHOOKS_H
#define TESTS_PAGING_PMMTESTHOOKS_H

#include <cstdint>

int pmmTestAllocCount();
int pmmTestFreeCount();
void pmmTestResetCounts();
uint8_t pmmTestRefcount(void *physAddr);

#endif
