#include "fs.h"

void init_superblock(superblock *sb) {
    sb->total_blocks = TOTAL_BLOCKS;
    sb->inode_start = SUPERBLOCKS + BITMAP_INODE_BLOCKS + BITMAP_DATA_BLOCKS;
    sb->data_start = sb->inode_start + INODE_BLOCKS;
    sb->inode_bitmap_start = SUPERBLOCKS;
    sb->data_bitmap_start = SUPERBLOCKS + BITMAP_INODE_BLOCKS;
    sb->inode_count = INODE_BLOCKS * (BLOCK_SIZE / sizeof(inode));
}

void init_root_directory(disk_mem *dm)
{
    dm->i_bmap[0] = 1;
    dm->d_bmap[0] = 1;

    dir* root_dir = (dir*)malloc(sizeof(dir));
    for(int i = 0; i < (BLOCK_SIZE / sizeof(dir_entry)); i++)
    {
        root_dir->data[i].inum = -1;
    }

    int data_block_ind = 0;
    dm->block_list[data_block_ind] = (dir*)root_dir;
    

    inode* root_inode = (inode*)malloc(sizeof(inode));
    root_inode->uid = 0;
    root_inode->mode = 1;
    root_inode->size = sizeof(dir);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    root_inode->mtime = tv.tv_sec;
    root_inode->links_count = 0;
    root_inode->direct_blocks = (uint32_t*)malloc(sizeof(uint32_t) * DIRECT_BLOCKS);
    root_inode->direct_blocks[0] = 0;
    for(int i = 1; i < DIRECT_BLOCKS; i++)
    {
        root_inode->direct_blocks[i] = -1;
    } 
    root_inode->indirect_block = (uint32_t*)malloc(sizeof(uint32_t) * INDIRECT_BLOCKS);
    for(int i = 0; i < INDIRECT_BLOCKS; i++)
    {
        root_inode->indirect_block[i] = -1;
    }

    root_dir->data[0].inum = 0;
    root_dir->data[0].reclen = 1;
    root_dir->data[0].strlen = 1;
    strcpy(root_dir->data[0].name, ".");
}


void init_disk_mem(disk_mem* dm)
{
    superblock* sb = (superblock*)malloc(sizeof(superblock));
    init_superblock(sb);
    
    uint8_t* i_mp = (uint8_t*)calloc(BITMAP_INODE_BLOCKS * BLOCK_SIZE, sizeof(uint8_t));
    uint8_t* d_mp = (uint8_t*)calloc(BITMAP_DATA_BLOCKS * BLOCK_SIZE, sizeof(uint8_t));

    
    inode** inode_ls = (inode**)malloc(sizeof(inode*) * INODE_BLOCKS);
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        inode_ls[i] = (inode*)malloc(sizeof(inode));
    }
    void** block_ls = (void**)malloc(sizeof(void*) * DATA_BLOCKS);
    for(int i = 0; i < DATA_BLOCKS; i++)
    {
        block_ls[i] = NULL;
    }
    for(int i = 0; i < INODE_BLOCKS; i++) {
        inode_ls[i] = NULL; 
    }

    dm->disk_superblock = sb;
    dm->i_bmap = i_mp;
    dm->d_bmap = d_mp;
    dm->inode_list = inode_ls;
    dm->block_list = block_ls;
}


int add_new_file_to_directory( // 2- no such dir, 0 - no space in a dir, 1 - success
    disk_mem* dm, 
    uint32_t *inode_number_of_dir, 
    uint32_t *new_inode,
    uint16_t *reclen,
    char *name
    )
{
    dir* curr_dir = NULL;
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        if(dm->inode_list[i] != NULL && dm->inode_list[i]->uid == *inode_number_of_dir)
        {
            int address_curr_dir = dm->inode_list[i]->direct_blocks[0];
            curr_dir = dm->block_list[address_curr_dir];
            break;
        }
    }
    if(NULL == curr_dir) return 2;

    int index_for_new_entry = -1;

    for(int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        if(curr_dir->data[i]->inum == -1)
        {
            index_for_new_entry = i;
            break;
        }

    }
    if(index_for_new_entry == -1)
    {
        return 0;
    }
    curr_dir->data[index_for_new_entry]->inum = *new_inode;
    curr_dir->data[index_for_new_entry]->reclen = *reclen;
    strcpy(curr_dir->data[index_for_new_entry]->name, name);

    return 1;
}


// void print_all_inodes(disk_mem* dm)
// {
    
// }

// void print_all_blocks(disk_mem* dm)
// {
    
// }

// void print_all_dir_entries(disk_mem* dm, uint32_t *inode_number_of_dir)
// {
    
// }


