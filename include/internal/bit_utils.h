#ifndef FILESYSTEM_BIT_UTILS_H
#define FILESYSTEM_BIT_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

inline static bool is_bit_set(uint32_t reg, uint32_t bit) {
  return (reg >> bit) & 0x1;
}

inline static uint32_t extract_bit_range(uint32_t reg, uint32_t msb, uint32_t lsb) {
  const uint64_t bits = msb - lsb + 1ULL;
  const uint64_t mask = (1ULL << bits) - 1ULL;
  assert(msb >= lsb);
  return (reg >> lsb) & mask;
}

#endif // FILESYSTEM_BIT_UTILS_H
