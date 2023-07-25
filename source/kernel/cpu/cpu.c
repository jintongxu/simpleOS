#include "cpu/cpu.h"
#include "os_cfg.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "ipc/mutex.h"


static segment_desc_t gdt_table[GDT_TABLE_SIZE];
static mutex_t mutex;


void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t * desc = gdt_table + (selector >> 3);

    if (limit > 0xFFFFF) {
        // 如果limit太大，就把G位置1
        attr |= 0x8000;
        limit /= 0x1000;
    }

    desc->limit15_0 = limit & 0xFFFF;
    desc->base15_0 = base & 0xFFFF;
    desc->base23_16 = (base >> 16) & 0xFF;
    desc->attr = attr | (((limit >> 16) & 0xF) << 8);
    desc->base31_24 = (base >> 24) & 0xFF;
}


void gate_desc_set (gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr) {
    desc->offset15_0 = offset & 0xFFFF;
    desc->selector = selector;
    desc->attr = attr;
    desc->offset31_16 = (offset >> 16) & 0xFFFF;
}

int gdt_alloc_desc() {
    mutex_lock(&mutex);

    for (int i = 1; i < GDT_TABLE_SIZE; i ++ ) {
        segment_desc_t * desc = gdt_table + i;
        if (desc->attr == 0) {
            mutex_unlock(&mutex);
            return i * sizeof(segment_desc_t);
        }
    }

    mutex_unlock(&mutex);

    return -1;
}


void gdt_free_sel(int sel) {
    mutex_lock(&mutex);
    gdt_table[sel/sizeof(segment_desc_t)].attr = 0;
    mutex_unlock(&mutex);
}


void init_gdt (void) {
    for (int i = 0; i < GDT_TABLE_SIZE; i ++ ) {
        segment_desc_set(i << 3, 0, 0, 0);
    }


    segment_desc_set(KERNEL_SELECTOR_DS, 0x00000000, 0xFFFFFFFF,
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA
        | SEG_TYPE_RW | SEG_D | SEG_G);

    segment_desc_set(KERNEL_SELECTOR_CS, 0x00000000, 0xFFFFFFFF,
        SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE
        | SEG_TYPE_RW | SEG_D | SEG_G);

    lgdt((uint32_t)gdt_table, sizeof(gdt_table));


}
void cpu_init (void) {
    mutex_init(&mutex);
    init_gdt();
}


void switch_to_tss (int tss_sel) {
    far_jump(tss_sel, 0);
}