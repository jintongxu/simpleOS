#include "loader.h"
#include "comm/elf.h"
/*
LEA48方式读取磁盘
sector: 从哪个扇区读取
sector_count： 读取多少个
buf: 放到哪里
*/
static void read_disk(int sector, int sector_count, uint8_t * buf) {
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}


static uint32_t reload_elf_file (uint8_t * file_buffer) {
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != 0x7F) || (elf_hdr->e_ident[1] != 'E')
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
            return 0;
    }

    for (int i = 0; i < elf_hdr->e_phnum; i ++) {
        Elf32_Phdr * phdr = (Elf32_Phdr * )(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j ++ ) {
            *dest++ = *src++;
        }
        dest = (uint8_t *)phdr->p_paddr + phdr->p_filesz;
        for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j ++ ) {
            *dest++ = 0;
        }
    }

    return elf_hdr->e_entry;

}

static void die(int code) {
    for (;;) {}
}

#define PDE_P       (1 << 0)
#define PDE_W       (1 << 1)
#define PDE_PS      (1 << 7)
#define CR4_PSE     (1 << 4)
#define CR0_PG      (1 << 31)
// 开启分页机制
void enable_page_mode(void) {
    static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {    // 以4KB方式对齐
        [0] = PDE_P | PDE_W | PDE_PS | 0
    };

    uint32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE);

    write_cr3((uint32_t)page_dir);
    write_cr0(read_cr0() | CR0_PG);
}

void load_kernel (void) {
    read_disk(100, 500, (uint8_t *)SYS_KERNEL_LOAD_ADDR);

    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);
    if (kernel_entry == 0) {
        die(-1);
    }

    // 打开分页机制
    enable_page_mode();

    // 将参数传递到init/start.S的_start处
    ((void (*)(boot_info_t *))kernel_entry)(&boot_info);

}