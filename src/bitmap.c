#include "bitmap.h"

bitmap_t *bitmap_create(uint32_t num_blocks) {
  bitmap_t *bitmap = malloc(sizeof(bitmap_t));

  bitmap->map = (uint8_t *) calloc(num_blocks, 1);
  assert(bitmap->map);

  bitmap->num_bytes = num_blocks;
  bitmap->num_bits = bitmap->num_bytes * 8;
  for (int i = 0; i < bitmap->num_bytes; i++) {
    bitmap->map[i] = 0xFF;
  }
  return bitmap;
}

void bitmap_set_bits(bitmap_t *bitmap, int32_t val, uint32_t nbits, uint32_t index) {
  uint32_t bit = 0;

  for (uint32_t i = 0; i < bitmap->num_bytes; i++) {
    uint8_t mask = 1;
    for (uint32_t j = 0; j < 8; j++) {
      if (bit < index) {
        bit++;
        mask = mask << 1;
        continue;
      }
      if (bit == index + nbits) {
        return;
      }
      if (val) {
        bitmap->map[i] = bitmap->map[i] | mask;
      } else {
        bitmap->map[i] = bitmap->map[i] & (~mask);
      }
    }
  }
}

int32_t bitmap_get_bit(bitmap_t *bitmap, uint32_t index) {
  uint32_t bit = 0;

  for (uint32_t i = 0; i < bitmap->num_bytes; i++) {
    uint8_t mask = 1;
    for (uint32_t j = 0; j < 8; j++) {
      if (bit == index) {
        return bitmap->map[i] & mask ? 1 : 0;
      }
      bit++;
      mask = mask << 1;
    }
  }
  return -1;
}

int32_t bitmap_get_bit_run(bitmap_t *bitmap, uint32_t size) {
  uint8_t current_size = 0;
  uint8_t bit = 0;

  for (uint32_t i = 0; i < bitmap->num_bytes; i++) {
    uint32_t mask = 1;
    for (uint32_t j = 0; j < 8; j++) {
      if (bitmap->map[i] & mask) {
        current_size++;
        if (current_size == size)
          return bit - size + 1;
      } else {
        current_size = 0;
      }
      bit++;
      mask = mask << 1;
    }
  }
  return -1;
}

void bitmap_show(bitmap_t *bitmap) {
  uint32_t bit = 0;
  uint8_t mask;
  for (int i = 0; i < bitmap->num_bytes; i++) {
    mask = 1;
    printf("byte[%u]=%x\n", i, bitmap->map[i]);
    for (int j = 0; j < 8; j++) {
      if (bitmap->map[i] & mask) {
        printf("1");
      } else {
        printf("0");
      }
      bit++;
      mask = mask << 1;
    }
    printf("\n\n");
  }
}
