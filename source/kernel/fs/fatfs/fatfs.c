#include "fs/fs.h"
#include "fs/fatfs/fatfs.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "core/memory.h"
#include "tools/klib.h"

/**
 * @brief 缓存读取磁盘数据，用于目录的遍历等
*/
static int bread_sector (fat_t * fat, int sector) {
    if (sector == fat->curr_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    if (cnt == 1) {
        fat->curr_sector = sector;
        return 0;
    }

    return -1;
}


/**
 * @brief 在root目录中读取diritem
*/
static diritem_t * read_dir_entry (fat_t * fat, int index) {
    if ((index < 0) || (index >= fat->root_ent_cnt)) {
        return (diritem_t *)0;
    }

    int offset = index * sizeof(diritem_t);     // 在根目录处的字节偏移
    int sector = fat->root_start + offset / fat->bytes_per_sec; 
    int err = bread_sector(fat, sector);
    if (err < 0) {
        return (diritem_t *)0;
    }


    return (diritem_t *)(fat->fat_buffer + offset % fat->bytes_per_sec);

}


/**
 * @brief 获取文件类型
 */
file_type_t diritem_get_type (diritem_t * diritem) {
    file_type_t type = FILE_UNKNOWN;

    // 长文件名和volum id
    if (diritem->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }
    
    if ((diritem->DIR_Attr & DIRITEM_ATTR_LONG_NAME) == 0xF) {
        return FILE_UNKNOWN;
    }

    return diritem->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
    
}


/**
 * @brief 获取diritem中的名称，转换成合适
*/
void diritem_get_name (diritem_t * item, char * dest) {
    char * c = dest;
    char * ext = (char *)0;

    kernel_memset(dest, 0, 12);
    for (int i = 0; i < 11; i++) {
        if (item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }

        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

     // 没有扩展名的情况
    if (ext && (ext[1] == '\0')) {
        ext[0] = '\0';
    }
}


/**
 * @brief 转换文件名为diritem中的短文件名，如a.txt 转换成a      txt
 */
static void to_sfn (char * dest, const char * src) {
    kernel_memset(dest, ' ', 11);

    char * curr = dest;
    char * end = dest + 11;
    while (*src && (curr < end)) {
        char c = *src++;
        switch (c) {
            case '.':
                curr = dest + 8;
                break;
            default:
                if ((c >= 'a') && (c <= 'z')) {
                    c = c - 'a' + 'A';
                }
                *curr++= c;
                break;
        }
    }

}


/**
 * @brief 判断item项是否与指定的名称相匹配
 */
int diritem_name_match (diritem_t * item, const char * path) {
    char buf[11];
    to_sfn(buf, path);
    return kernel_memcmp(buf, item->DIR_Name, 11) == 0;

}


/**
 * @brief 从diritem中读取相应的文件信息
 */
static void read_from_diritem (fat_t * fat, file_t * file, diritem_t * item, int index) {
    file->type = diritem_get_type(item);
    file->size = item->DIR_FileSize;
    file->pos = 0;
    file->p_index = index;
    file->sblk = (item->DIR_FstClusHI << 16) | (item->DIR_FstClusL0);
    file->cblk = file->sblk;
}


/**
 * 检查指定簇是否可用，非占用或坏簇
 */
int cluster_is_valid (cluster_t cluster) {
    return (cluster < FAT_CLUSTER_INVALID) && (cluster >= 0x2);
}


/**
 * 获取指定簇的下一个簇
 */
int cluster_get_next (fat_t * fat, cluster_t curr) {
    if (!cluster_is_valid(curr)) {
        return FAT_CLUSTER_INVALID;
    }

    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sec;   // 在 FAT 表上的扇区号
    int off_sector = offset % fat->bytes_per_sec;       // 在扇区中的偏移

    // 如果超过最大号
    if (sector >= fat->tbl_sectors) {
        log_printf("cluster too big: %d", curr);
        return FAT_CLUSTER_INVALID;
    }

    int err = bread_sector(fat, fat->tbl_start + sector);
    if (err < 0) {
        return FAT_CLUSTER_INVALID;
    }

    return *(cluster_t *)(fat->fat_buffer + off_sector);
}

/**
 * @brief 移动文件指针
 */
static int move_file_pos (file_t * file, fat_t * fat, uint32_t move_bytes, int expand) {
    uint32_t c_offset = file->pos % fat->cluster_byte_size;     // 在簇中的偏移
    // 当前偏移加上读取数据大小 超出 当前簇
    if (c_offset + move_bytes >= fat->cluster_byte_size) {
        cluster_t next = cluster_get_next(fat, file->cblk);  // 通过查fat表找到下一个簇
        if (next == FAT_CLUSTER_INVALID) {
            return -1;
        }
        file->cblk = next;
    }

    file->pos += move_bytes;
    return 0;
}



/**
 * @brief 挂载fat文件系统
*/
int fatfs_mount (struct _fs_t * fs, int major, int minor) {
    // 打开设备
    int dev_id = dev_open(major, minor, (void *)0);
    if (dev_id < 0) {
        log_printf("open disk failed. major: %x, minor: %x", major, minor);
        return -1;
    }

    // 读取dbr扇区并进行检查
    dbr_t * dbr = (dbr_t *)memory_alloc_page();
    if (!dbr) {
        log_printf("mount failed.: can`t alloc buf");
        goto mount_failed;
    }

    // 这里需要使用查询的方式来读取，因为此时多进程还没有跑起来，只在初始化阶段？
    int cnt = dev_read(dev_id, 0, (char *)dbr, 1);
    if (cnt < 1) {
        log_printf("read dbr failed");
        goto mount_failed;
    }

    // 解析DBR参数，解析出有用的参数
    fat_t * fat = &fs->fat_data;
    fat->fat_buffer = (uint8_t *)dbr;
    fat->bytes_per_sec = dbr->BPB_BytsPerSec;
    fat->tbl_start = dbr->BPB_RsvdSecCnt;
    fat->tbl_sectors = dbr->BPB_FATSz16;
    fat->tbl_cnt = dbr->BPB_NumFATs;
    fat->root_ent_cnt = dbr->BPB_RootEntCnt;
    fat->sec_per_cluster = dbr->BPB_SecPerClus;
    fat->root_start = fat->tbl_start + fat->tbl_sectors * fat->tbl_cnt;
    fat->data_start = fat->root_start + fat->root_ent_cnt * 32 / SECTOR_SIZE;
    fat->cluster_byte_size = fat->sec_per_cluster * dbr->BPB_BytsPerSec;
    fat->fs = fs;
    mutex_init(&fat->mutex);
    fs->mutex = &fat->mutex;

    fat->curr_sector = -1;

    // 简单检查是否是fat16文件系统, 可以在下边做进一步的更多检查。此处只检查做一点点检查
    if (fat->tbl_cnt != 2) {
        log_printf("fat table error: major: %x, minor: %x", major, minor);
        goto mount_failed;
    }


    if (kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("not a fat filesystem: major: %x, minor: %x", major, minor);
        goto mount_failed;
    }

    // 记录相关的打开信息
    fs->type = FS_FAT16;
    fs->data = &fs->fat_data;
    fs->dev_id = dev_id;
    return 0;

mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
    return -1;
}

// 卸载fatfs文件系统
void fatfs_unmount (struct _fs_t * fs) {
    fat_t * fat = (fat_t *)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}


// 打开指定的文件
int fatfs_open (struct _fs_t * fs, const char * path, file_t * file) {
    fat_t * fat = (fat_t *)fs->data;
    diritem_t * file_item = (diritem_t *)0;
    int p_index = -1;

    // 遍历根目录的数据区，找到已经存在的匹配项
    for (int i = 0; i < fat->root_ent_cnt; i++) {
        diritem_t * item = read_dir_entry(fat, i);
        if (item == (diritem_t *)0) {
            return -1;
        }

        // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            continue;
        }

        // 找到要打开的目录
        if (diritem_name_match(item, path)) {
            file_item = item;
            p_index = i;
            break;
        }
    }

    if (file_item) {
        read_from_diritem(fat, file, file_item, p_index);
        return 0;
    }


    return -1;
}

int fatfs_read (char * buf, int size, file_t * file) {
    fat_t * fat = (fat_t *)file->fs->data;


    uint32_t nbytes = size;
    if (file->pos + nbytes > file->size) {
        nbytes = file->size - file->pos;
    }

    uint32_t total_read = 0;

    while (nbytes > 0) {
        uint32_t curr_read = nbytes;
        uint32_t cluster_offset = file->pos % fat->cluster_byte_size;   // 在该簇中的偏移
        uint32_t start_sector = fat->data_start + (file->cblk - 2) * fat->sec_per_cluster;  // 当前的扇区号

        // 在这个簇的开头开始读并且当前读取的数据量正好是一个簇
        if ((cluster_offset == 0) && (nbytes == fat->cluster_byte_size)) {
            int err = dev_read(fat->fs->dev_id, start_sector, buf, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }


            curr_read = fat->cluster_byte_size;
        } else {
            // 判断读取数据是否超过当前簇
            if (cluster_offset + curr_read > fat->cluster_byte_size) {
                curr_read = fat->cluster_byte_size - cluster_offset;
            }

            fat->curr_sector = -1;
            int err = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }

            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);
        }

        buf += curr_read;
        nbytes -= curr_read;
        total_read += curr_read;


        int err = move_file_pos(file, fat, curr_read, 0);
        if (err < 0) {
            return total_read;
        }

    }


    return total_read;
}

int fatfs_write (char * buf, int size, file_t * file) {
    return 0;
}

void fatfs_close (file_t * file) {

}

int fatfs_seek (file_t * file, uint32_t offset, int dir) {
    return -1;
}

int fatfs_stat (file_t * file, struct stat *st) {
    return -1;
}


int fatfs_opendir (struct _fs_t * fs, const char * name, DIR * dir) {
    dir->index = 0;
    return 0;
}

/**
 * @brief 读取一个目录项
 */
int fatfs_readdir (struct _fs_t * fs, DIR * dir, struct dirent * dirent) {
    fat_t * fat = (fat_t *)fs->data;

    // 做一些简单的判断，检查
    while (dir->index < fat->root_ent_cnt) 
    {
        diritem_t * item = read_dir_entry(fat, dir->index);
        if (item == (diritem_t *)0) {
            return -1;
        }

        // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] != DIRITEM_NAME_FREE) {
            file_type_t type = diritem_get_type(item);
            if ((type == FILE_NORMAL) || (type == FILE_DIR)) {
                dirent->size = item->DIR_FileSize;
                dirent->type = type;
                diritem_get_name(item, dirent->name);
                dirent->index = dir->index++;
                return 0;
            }
        }

        dir->index++;
    }
    
    return -1;
}

int fatfs_closedir (struct _fs_t * fs, DIR * dir) {
    return 0;
}


fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .close = fatfs_close,
    .seek = fatfs_seek,
    .stat = fatfs_stat,

    .opendir = fatfs_opendir,
    .readdir = fatfs_readdir,
    .closedir = fatfs_closedir,
};