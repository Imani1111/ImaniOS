#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <stddef.h>

#define VDISK_SECTOR_SIZE 512
#define VDISK_SECTORS 2048

#define MAGIC_NUMBER 0x11111111
#define MAX_INODES 256
#define INODES_PER_SECTOR (VDISK_SECTOR_SIZE / sizeof(struct Inode))

#define TYPE_FREE 0
#define TYPE_FILE 1
#define TYPE_DIR  2

struct Superblock{
    uint32_t magic_number;
    uint32_t total_sectors;
    uint32_t inode_count;
    uint8_t padding[500];
};

struct Inode{
    char entry_name[48];
    uint32_t size_of_entry_in_bytes;
    uint32_t start_block;
    uint8_t type;
    uint8_t padding[7];
};

void vdisk_read_sector(uint16_t sector_num, void* buffer);
void vdisk_write_sector(uint16_t sector_num, void* buffer);
void fs_format(void);

void fs_read_inode(uint32_t inode_num, struct Inode* out_inode);
void fs_write_inode(uint32_t inode_num, struct Inode* in_inode);
int32_t fs_find_free_inode(void);
int32_t fs_find_free_block(void);

int32_t fs_create_entry(char* name, uint8_t type);
int32_t fs_create_file(const char* file_name);
int32_t fs_create_dir(const char* dir_name);

int32_t fs_find_inode_by_name(const char* name);
int32_t fs_write_file(const char* name, const void* data, uint32_t size);
int32_t fs_read_file(const char* name, void* output_buffer);
int32_t fs_delete_entry(const char* name);
void fs_list_directory(void);

#endif
