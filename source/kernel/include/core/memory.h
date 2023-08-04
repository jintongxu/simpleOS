#ifndef MEMORY_H
#define MEMORY_H

#include "comm/types.h"
#include "tools/bitmap.h"
#include "ipc/mutex.h"
#include "comm/boot_info.h"

#define MEM_EBDA_START              0x00080000
#define MEM_EXT_START               (1024*1024)     // 1MB
#define MEM_PAGE_SIZE               4096    // 4KB
#define MEMORY_TASK_BASE            0x80000000
#define MEM_EXT_END                 (127 * 1024 * 1024 - 1)


// 内存块
typedef struct _addr_alloc_t {
    mutex_t mutex;
    bitmap_t bitmap;

    uint32_t page_size;
    uint32_t start;
    uint32_t size;

}addr_alloc_t;

typedef struct _memory_map_t {
    void * vstart;    // 线性地址
    void * vend;
    void * pstart;    // 物理地址中
    uint32_t perm;      // 特权属性
}memory_map_t;

void memory_init (boot_info_t * boot_info);
uint32_t memory_create_uvm (void);

int memory_alloc_page_for (uint32_t addr, uint32_t size, int perm);
uint32_t memory_alloc_page (void);               // 分配一页内存
void memory_free_page (uint32_t addr);          // 释放地址

#endif