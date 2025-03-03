#ifndef FS_H
#define FS_H

#include <stdint.h>

#define BLOCK_SIZE 4096  
#define INODE_BLOCKS 5
#define DATA_BLOCKS 56
#define BITMAP_INODE_BLOCKS 1
#define BITMAP_DATA_BLOCKS 1
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
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t mtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t *direct_blocks; 
    uint32_t *indirect_block;
} inode;

typedef struct dir_entry {
    uint32_t inum;
    uint16_t reclen;
    uint8_t strlen;
    char name[64]; 
} dir_entry;

typedef struct block_entry {
    uint32_t data[BLOCK_SIZE / sizeof(uint32_t)];
} block;

struct disk_mem {
    superblock disk_superblock;
    uint8_t *i_bmap;
    uint8_t *d_bmap;
    inode *inode_list;
    void **block_list;
};

void init_superblock(superblock *sb);

#endif // FS_H
