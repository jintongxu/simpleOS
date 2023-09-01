/**
 * 位图数据结构
 */
#ifndef BITMAP_H
#define BITPAM_H

#include "comm/types.h"

/**
 * @brief 位图数据结构
 */
typedef struct _bitmap_t {
    int bit_count;      // 位的数据
    uint8_t * bits;     // 位空间

}bitmap_t;

int bitmap_byte_count(int bit_count);
void bitmap_init (bitmap_t * bitmap, uint8_t * bits, int count, int init_bit);
int bitmap_get_bit (bitmap_t * bitmap, int index);
void bitmap_set_bit (bitmap_t * bitmap, int index, int count,int bit);
int bitmap_is_set (bitmap_t * bitmap, int index);   // 判断某一位是否置0还是置1
int bitmap_alloc_nbits (bitmap_t * bitmap, int bit, int count);     // 找连续的多个位为bit的位进行取反

#endif