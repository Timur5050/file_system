#include "fs.h"

void init_superblock(superblock *sb) {
    sb->total_blocks = TOTAL_BLOCKS;
    sb->inode_start = SUPERBLOCKS + BITMAP_INODE_BLOCKS + BITMAP_DATA_BLOCKS;
    sb->data_start = sb->inode_start + INODE_BLOCKS;
    sb->inode_bitmap_start = SUPERBLOCKS;
    sb->data_bitmap_start = SUPERBLOCKS + BITMAP_INODE_BLOCKS;
    sb->inode_count = INODE_BLOCKS * (BLOCK_SIZE / sizeof(inode));
}
