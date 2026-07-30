#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include <cstddef>

#include <sys/StatsSnapshot.h>

class Stats {
public:
  static constexpr size_t RING_SIZE = 64;

  static void snapshot();
  static bool getLatest(StatsSnapshot &out);
  static size_t ringEntryCount();
};

#endif
