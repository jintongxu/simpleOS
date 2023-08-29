#ifndef DISK_H
#define DISK_H

#include "comm/types.h"

#define DISK_NAME_SIZE  32              // 分区名称
#define PART_NAME_SIZE  32              // 磁盘名称大小
#define DISK_PRIMARY_PART_CNT   (4+1)   // 主分区数量最多才4个      加1是把4个分区看成一个大分区进行描述
#define DISK_CNT        2               // 磁盘的数量
#define DISK_PER_CHANNEL            2   // 每通道磁盘数量
#define MBR_PRIMARY_PART_NR     4       // 4个分区表

// 只考虑支持主总线primary bus
#define IOBASE_PRIMARY              0x1F0
#define	DISK_DATA(disk)				(disk->port_base + 0)		// 数据寄存器
#define	DISK_ERROR(disk)			(disk->port_base + 1)		// 错误寄存器
#define	DISK_SECTOR_COUNT(disk)		(disk->port_base + 2)		// 扇区数量寄存器
#define	DISK_LBA_LO(disk)			(disk->port_base + 3)		// LBA寄存器
#define	DISK_LBA_MID(disk)			(disk->port_base + 4)		// LBA寄存器
#define	DISK_LBA_HI(disk)			(disk->port_base + 5)		// LBA寄存器
#define	DISK_DRIVE(disk)			(disk->port_base + 6)		// 磁盘或磁头？
#define	DISK_STATUS(disk)			(disk->port_base + 7)		// 状态寄存器
#define	DISK_CMD(disk)				(disk->port_base + 7)		// 命令寄存器      

// 状态寄存器
#define DISK_STATUS_ERR             (1 << 0)    // 发生了错误
#define DISK_STATUS_DRQ             (1 << 3)    // 准备好接受数据或者输出数据
#define DISK_STATUS_DF              (1 << 5)    // 驱动错误
#define DISK_STATUS_BUSY            (1 << 7)    // 正忙

#define DISK_DRIVE_BASE             0xE0

// ATA命令
#define DISK_CMD_IDENTIFY   0xEC        // IDENTIFY命令
#define DISK_CMD_READ       0x24        // 读命令
#define DISK_CMD_WRITE      0x34        // 写命令


#pragma pack(1)
// MBR的分区表项类型
typedef struct _part_item_t {
    uint8_t boot_active;               // 分区是否活动
	uint8_t start_header;              // 起始header
	uint16_t start_sector : 6;         // 起始扇区  6位
	uint16_t start_cylinder : 10;	    // 起始磁道 10位
	uint8_t system_id;	                // 文件系统类型
	uint8_t end_header;                // 结束header
	uint16_t end_sector : 6;           // 结束扇区  6位
	uint16_t end_cylinder : 10;        // 结束磁道  10位
	uint32_t relative_sectors;	        // 相对于该驱动器开始的相对扇区数
	uint32_t total_sectors;            // 总的扇区数
}part_item_t;


// MBR区域描述结构
typedef struct _mbr_t {
    uint8_t code[446];
    part_item_t part_item[MBR_PRIMARY_PART_NR];
    uint8_t boot_sig[2];
}mbr_t;
#pragma pack()

// 分区类型
typedef struct _partinfo_t {
    char name[PART_NAME_SIZE];
     struct _disk_t * disk;      // 所属的磁盘

    enum {
        FS_INVALID = 0x00,  // 无效文件系统类型
        FS_FAT16_0 = 0x6,   // FAT16文件系统类型
        FS_FAT16_1 = 0xE,
    }type;

    int start_sector;   // 起始扇区
    int total_sector;   // 总扇区
}partinfo_t;


typedef struct _disk_t {
    char name[DISK_NAME_SIZE];  // 磁盘名称

    enum {
        DISK_MASTER = (0 << 4),        // 主设备
        DISK_SLAVE = (1 << 4),      // 从设备
    }drive;

    uint16_t port_base;     // 端口起始地址  方便识别主从线

    int sector_size;        // 块大小
    int sector_count;       // 总扇区数量
    partinfo_t partinfo[DISK_PRIMARY_PART_CNT];   // 分区表, 包含一个描述整个磁盘的假分区信息

}disk_t;


void disk_init (void);  // 磁盘初始化及检测


void exception_handler_ide_primary (void);


#endif