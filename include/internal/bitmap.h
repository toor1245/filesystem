#ifndef FILESYSTEM_BITMAP_H
#define FILESYSTEM_BITMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct {
  uint8_t *map;
  uint32_t num_bits;
  uint32_t num_bytes;
} bitmap_t;

bitmap_t *bitmap_create(uint32_t num_blocks);

void bitmap_set_bits(bitmap_t *bitmap, int32_t val, uint32_t nbits, uint32_t index);

int32_t bitmap_get_bit(bitmap_t *bitmap, uint32_t index);

int32_t bitmap_get_bit_run(bitmap_t *bitmap, uint32_t size);

void bitmap_show(bitmap_t *bitmap);

#endif // FILESYSTEM_BITMAP_H
