#ifndef BITMAP_H
#define BITPAM_H

#include "comm/types.h"

typedef struct _bitmap_t {
    int bit_count;
    uint8_t * bits;

}bitmap_t;

int bitmap_byte_count(int bit_count);
void bitmap_init (bitmap_t * bitmap, uint8_t * bits, int count, int init_bit);

#endif