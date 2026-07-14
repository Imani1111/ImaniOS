#include <file_system.h>
#include <string.h>
#include <vga.h>

static uint32_t ramdisk[VDISK_SECTOR_SIZE*VDISK_SECTORS];
static uint32_t total_inodes = 0;
//static int total_size = 0;

void vdisk_read_sector(uint16_t sector_num, void* buffer)
{
    if (sector_num >= VDISK_SECTORS) return;
    my_memcpy(buffer, &ramdisk[sector_num * VDISK_SECTOR_SIZE], VDISK_SECTOR_SIZE);
}

void vdisk_write_sector(uint16_t sector_num, void* buffer)
{
    if (sector_num >= VDISK_SECTORS) return;
    my_memcpy(&ramdisk[sector_num * VDISK_SECTOR_SIZE], buffer, VDISK_SECTOR_SIZE);
}

void fs_format(void)
{
    static uint8_t sector_buffer[VDISK_SECTOR_SIZE];
    my_memset(sector_buffer, 0, VDISK_SECTOR_SIZE);

    static struct Superblock superblock;
    my_memset(&superblock, 0, sizeof(struct Superblock));
    
    superblock.magic_number = MAGIC_NUMBER;
    superblock.total_sectors = VDISK_SECTORS;
    superblock.inode_count = MAX_INODES;
    vdisk_write_sector(0, &superblock);

    
    my_memcpy(sector_buffer, &superblock, sizeof(struct Superblock));

    vdisk_write_sector(0, sector_buffer);
    my_memset(sector_buffer, 0, VDISK_SECTOR_SIZE);

    for(uint32_t i = 1; i <= 32; i++){
        vdisk_write_sector(i, sector_buffer);
    }
}

void fs_read_inode(uint32_t inode_num, struct Inode* out_inode)
{
    if (inode_num >= MAX_INODES) return;

    uint32_t sector_number = 1 + (inode_num / INODES_PER_SECTOR);
    uint32_t offset = (inode_num % INODES_PER_SECTOR) * sizeof(struct Inode);

    uint8_t sector_buffer[VDISK_SECTOR_SIZE];
    vdisk_read_sector(sector_number, sector_buffer);

    my_memcpy(out_inode, &sector_buffer[offset], sizeof(struct Inode));
}

void fs_write_inode(uint32_t inode_num, struct Inode* in_inode)
{
    if (inode_num >= MAX_INODES) return;
    uint32_t sector_number = 1 + (inode_num / INODES_PER_SECTOR);
    uint32_t offset = (inode_num % INODES_PER_SECTOR) * sizeof(struct Inode);

    uint8_t sector_buffer [VDISK_SECTOR_SIZE];
    vdisk_read_sector(sector_number, sector_buffer);

    my_memcpy(&sector_buffer[offset], in_inode, sizeof(struct Inode));

    vdisk_write_sector(sector_number, sector_buffer);
}

int32_t fs_find_free_inode(void)
{
    struct Inode temp_inode;
    for(int i = 1; i < MAX_INODES; i++){
        fs_read_inode(i, &temp_inode);
        if (temp_inode.type == TYPE_FREE){
            return i;
        }
    }
    return -1;
}

int32_t fs_find_free_block(void)
{
    for (uint32_t block = 33; block < VDISK_SECTORS; block++){
        uint32_t block_used = 0;

        for (uint32_t i = 0; i < MAX_INODES; i++){
            struct Inode temp_inode;
            fs_read_inode(i, &temp_inode);
            if (temp_inode.type != TYPE_FREE && temp_inode.start_block == block){
                block_used = 1;
                break;
            }
        }

        if (!block_used){
            return block;
        }
    }
    return -1;
}

int32_t fs_create_entry(char* name, uint8_t type)
{
    if (my_strlen(name) > 48){
        return -1;
    }
    int32_t inode_num = fs_find_free_inode();
    if (inode_num < 0){
        return -1;
    }

    int32_t block_num = fs_find_free_block();
    if (block_num < 0){
        return -1;
    }
    
    struct Inode new_inode;
    my_memset(&new_inode, 0, sizeof(struct Inode));

    my_strncpy(new_inode.entry_name, name, 47);
    new_inode.entry_name[47] = '\0';
    new_inode.size_of_entry_in_bytes = 0;
    new_inode.start_block = block_num;
    new_inode.type = type;

    fs_write_inode(inode_num, &new_inode);
    
    uint8_t empty_sector[VDISK_SECTOR_SIZE];
    my_memset(empty_sector, 0, VDISK_SECTOR_SIZE);
    vdisk_write_sector(block_num, empty_sector);

    total_inodes++;

    return inode_num;
}

int32_t fs_create_file(const char* file_name)
{
    int32_t result = fs_create_entry(file_name, TYPE_FILE);
    if (result < 0){
        return -1;
    }else{
        return 0;
    }
}

int32_t fs_create_dir(const char* dir_name)
{
    int32_t result = fs_create_entry(dir_name, TYPE_DIR);
    if(result < 0){
        return -1;
    }else{
        return 0;
    }
}

int32_t fs_find_inode_by_name(const char* name)
{
    struct Inode temp_inode;
    for (uint32_t i = 0; i < MAX_INODES; i++){
        fs_read_inode(i, &temp_inode);
        if (temp_inode.type != TYPE_FREE && my_strcmp(temp_inode.entry_name, name) == 0){
            return i;
        }
    }
    return -1;
}

int32_t fs_write_file(const char* name, const void* data, uint32_t size)
{
    struct Inode file_inode;
    int32_t inode_num = fs_find_inode_by_name(name);

    if (size > VDISK_SECTOR_SIZE) return -1;
    if (inode_num < 0) return -1;

    fs_read_inode(inode_num, &file_inode);
    if (file_inode.type != TYPE_FILE) return -1;
 
    vdisk_write_sector(file_inode.start_block, data);
   
    file_inode.size_of_entry_in_bytes = size;
    fs_write_inode(inode_num, &file_inode);
   
    return 0;
}

int32_t fs_read_file(const char *name, void *output_buffer) {
    int32_t inode_num = fs_find_inode_by_name(name);
    if (inode_num < 0) return -1;

    struct Inode file_inode;
    fs_read_inode(inode_num, &file_inode);

    if (file_inode.type != TYPE_FILE) return -1;

    vdisk_read_sector(file_inode.start_block, output_buffer);

    return file_inode.size_of_entry_in_bytes;
}

int32_t fs_delete_entry(const char* name)
{
    struct Inode temp_inode;
    int32_t inode_num = fs_find_inode_by_name(name);
    if (inode_num < 0) return -1;

    my_memset(&temp_inode, 0, sizeof(temp_inode));
    fs_write_inode(inode_num, &temp_inode);

    total_inodes--;

    return 0;
}


void fs_list_directory(void) {
    struct Inode temp_inode;
    uint32_t total_size = 0;

    for (uint32_t i = 0; i < MAX_INODES; i++){
        fs_read_inode(i, &temp_inode);
        if (temp_inode.type == TYPE_FILE){
            total_size += temp_inode.size_of_entry_in_bytes;
        }
    }
    vga_puts("total: ");
    vga_set_text_attribute(VGA_ATTR_PURPLE_ON_BLACK);
    vga_print_int(total_size);
    vga_putc('\n');
    vga_set_default_text_attribute();
    vga_puts("Type    Size    Name\n");
    vga_puts("------------------------------------\n");
    
    if (total_inodes == 0){
        vga_set_text_attribute(VGA_ATTR_RED_ON_BLACK);
        vga_puts("You have not created any entry in imani's virtual file system!\n");
        vga_puts("To create an entry, do 'touch <filename>' or 'mkdir <dirname>'\n");
        vga_set_default_text_attribute();
    }

    for (uint32_t i = 0; i < MAX_INODES; i++) {
        fs_read_inode(i, &temp_inode);
        
        if (temp_inode.type != TYPE_FREE) {
            if (temp_inode.type == TYPE_DIR) {
                vga_puts("<DIR>   ");
            } else {
                vga_puts("FILE    ");
            }
            
            vga_set_text_attribute(VGA_ATTR_PURPLE_ON_BLACK);
            vga_print_int(temp_inode.size_of_entry_in_bytes);
            vga_set_default_text_attribute();
            vga_puts("Bytes       "); 

            vga_puts(temp_inode.entry_name);
            vga_putc('\n');
        }
    }
}
