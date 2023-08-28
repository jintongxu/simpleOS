#ifndef DISK_H
#define DISK_H

#include "comm/types.h"

#define DISK_NAME_SIZE  32              // 分区名称
#define PART_NAME_SIZE  32              // 磁盘名称大小
#define DISK_PRIMARY_PART_CNT   (4+1)   // 主分区数量最多才4个
#define DISK_CNT        2               // 磁盘的数量



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
    int sector_size;        // 块大小
    int sector_count;       // 总扇区数量
    partinfo_t partinfo[DISK_PRIMARY_PART_CNT];   // 分区表, 包含一个描述整个磁盘的假分区信息

}disk_t;


void disk_init (void);  // 磁盘初始化及检测


#endif