#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#define BLOCK_SIZE 4096  
#define INODE_BLOCKS 15
#define DATA_BLOCKS 110
#define BITMAP_INODE_BLOCKS INODE_BLOCKS
#define BITMAP_DATA_BLOCKS DATA_BLOCKS
#define SUPERBLOCKS 1
#define DIRECT_BLOCKS 8
#define INDIRECT_BLOCKS 8


#define TOTAL_BLOCKS (INODE_BLOCKS + DATA_BLOCKS + BITMAP_INODE_BLOCKS + BITMAP_DATA_BLOCKS + SUPERBLOCKS)

typedef struct superblock {
    uint32_t total_blocks;
    uint32_t inode_start;
    uint32_t data_start;
    uint32_t inode_bitmap_start;
    uint32_t data_bitmap_start;
    uint32_t inode_count; 
} superblock;

typedef struct inode {
    uint16_t mode; // dir or file (0 - file; 1 - dir)
    uint16_t uid; // id
    uint32_t size; // file size bytes
    uint32_t mtime; // хз шо це, вроді час створення але там з приколом   
    uint16_t links_count; // reference counter. how many processes reference to it
    uint32_t *direct_blocks; // indirect blocks pointers 
    uint32_t *indirect_block; // direct blocks pointers
} inode;
  
typedef struct dir_entry {
    uint32_t inum; // inode address of file or dir 
    uint16_t reclen; // тут кількість блоків зайнятих, якщо це файл. якшо це папка - кількість записів у директорію
    uint8_t strlen; // довжина назви запису
    char name[64]; // file or directory name
} dir_entry;

typedef struct dir {
    dir_entry data[BLOCK_SIZE / sizeof(dir_entry)];
} dir;

typedef struct block {
    uint32_t data[BLOCK_SIZE / sizeof(uint32_t)];
} block;

typedef struct disk_mem {
    superblock *disk_superblock;
    uint8_t *i_bmap;
    uint8_t *d_bmap;
    inode **inode_list;
    void **block_list;
} disk_mem;


uint32_t get_curr_time();
void init_superblock(superblock *sb);
void init_root_directory(disk_mem *dm);
void init_disk_mem(disk_mem* dm);
void print_all_inodes(disk_mem* dm);
void print_all_blocks(disk_mem* dm);
void print_all_dir_entries(disk_mem* dm, uint32_t inode_number_of_dir);
void print_i_bmap_and_d_bmap(disk_mem *dm);
void print_whole_dm(disk_mem *dm);
void print_whole_data_from_file(disk_mem *dm, uint32_t inode_number);
void add_file_to_directory(disk_mem *dm, uint16_t inode_number_of_dir, inode* file_inode, char* file_name);
uint16_t create_new_file(disk_mem *dm);
uint16_t create_file(disk_mem* dm, uint32_t inode_number_of_dir, char *file_name, uint32_t *file_data);
int8_t add_data_to_file(disk_mem *dm, uint16_t file_inode_number, uint32_t *file_data, uint32_t data_size);

#endif 
