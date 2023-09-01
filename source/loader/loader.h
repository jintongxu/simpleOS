/**
 * 二级加载部分，用于实现更为复杂的初始化、内核加载的工作。
 *
 */
#ifndef LOADER_H
#define LOADER_H

// 因为Cmake中定义了搜索路径，所以这里不用绝对路径，自动在source下寻找。
#include "comm/boot_info.h"
#include "comm/types.h"
#include "comm/cpu_instr.h"

// 保护模式入口函数，在start.asm中定义
void protect_mode_entry (void);

// 内存检测信息结构
typedef struct SMAP_entry {
 
	uint32_t BaseL; // base address uint64_t
	uint32_t BaseH;
	uint32_t LengthL; // length uint64_t
	uint32_t LengthH;
	uint32_t Type; // entry Type，值为1时表明为我们可用的RAM空间
	uint32_t ACPI; // extended, bit0=0时表明此条目应当被忽略
 
}__attribute__((packed)) SMAP_entry_t;


extern boot_info_t boot_info;

#endif