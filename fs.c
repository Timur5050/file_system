#include "fs.h"

uint32_t get_curr_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

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
    root_inode->mtime = get_curr_time();
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

    dm->inode_list[0] = root_inode;
}


void init_disk_mem(disk_mem* dm)
{
    superblock* sb = (superblock*)malloc(sizeof(superblock));
    init_superblock(sb);
    
    uint8_t* i_mp = (uint8_t*)calloc(BITMAP_INODE_BLOCKS * BLOCK_SIZE, sizeof(uint8_t));
    uint8_t* d_mp = (uint8_t*)calloc(BITMAP_DATA_BLOCKS * BLOCK_SIZE, sizeof(uint8_t));

    
    inode** inode_ls = (inode**)malloc(sizeof(inode*) * INODE_BLOCKS);
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
        if(curr_dir->data[i].inum == -1)
        {
            index_for_new_entry = i;
            break;
        }

    }
    if(index_for_new_entry == -1)
    {
        return 0;
    }
    curr_dir->data[index_for_new_entry].inum = *new_inode;
    curr_dir->data[index_for_new_entry].reclen = *reclen;
    strcpy(curr_dir->data[index_for_new_entry].name, name);

    return 1;
}


void print_all_inodes(disk_mem* dm)
{
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        inode* temp_inote = dm->inode_list[i];
        if(NULL == temp_inote)
        {
            continue;
        }
        printf("mode : %d\n", temp_inote->mode);
        printf("uid : %d\n", temp_inote->uid);
        printf("size : %d\n", temp_inote->size);
        printf("mtime : %d\n", temp_inote->mtime);
        printf("links_count : %d\n", temp_inote->links_count);
        printf("direct blocks : ");
        for(int j = 0; j < DIRECT_BLOCKS; j++)
        {
            printf("%d\t", temp_inote->direct_blocks[j]);
        }
        printf("\nindirect blocks : ");
        for(int j = 0; j < INDIRECT_BLOCKS; j++)
        {
            printf("%d\t", temp_inote->indirect_block[j]);
        }
        printf("\n");
    }
}

void print_all_blocks(disk_mem* dm)
{
    printf("---- block list ----\n");

    for(int block_index = 0; block_index < DATA_BLOCKS; block_index++)
    {
        void* block = dm->block_list[block_index];
        if(NULL == block)
        {
            continue;
        }

        inode* owner_inode = NULL;
        for(int j = 0; j < INDIRECT_BLOCKS; j++)
        {
            inode* temp_inode = dm->inode_list[j];

            if(NULL == temp_inode)
            {
                continue;
            }

            for(int k = 0; k < DIRECT_BLOCKS; k++)
            {
                if(temp_inode->direct_blocks[k] == block_index)
                {
                    owner_inode = temp_inode;
                    break;
                }
            }

            for(int k = 0; k < INDIRECT_BLOCKS; k++)
            {
                if(temp_inode->indirect_block[k] == block_index)
                {
                    owner_inode = temp_inode;
                    break;
                }
            }

            if(owner_inode)
            {
                break;
            }

        }

        if(owner_inode)
        {
            if(owner_inode->mode == 1) // dir
            {
                dir* directory = (dir*)block;
                printf("block %d: dir\n", block_index);

                for(int j = 0; j < (BLOCK_SIZE / sizeof(dir_entry)); j++)
                {
                    if(directory->data[j].inum != -1)
                    {
                        printf(" %d %s (inode: %d)\n", j, directory->data[j].name, directory->data[j].inum);
                    }
                }
            }
            else //file
            {
                printf("block %d: file data\n", block_index);
            }
        }
        else
        {
            printf("block %d: unknown type\n", block_index);
        }
    }
}

void print_all_dir_entries(disk_mem* dm, uint32_t inode_number_of_dir)
{
    inode *dir_inode = dm->inode_list[inode_number_of_dir];
    
    if(dir_inode->mode != 1)
    {
        printf("not a dir! \n");
    }
    else
    {
        for(int i = 0; i < DIRECT_BLOCKS; i++)
        {
            uint32_t curr_dir_inum = dir_inode->direct_blocks[i];
            if(curr_dir_inum == -1)
            {
                continue;
            }

            dir* curr_dir = (dir*)dm->block_list[curr_dir_inum];

            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry curr_dir_entry = curr_dir->data[j];

                if(curr_dir_entry.inum == -1)
                {
                    continue;
                }

                printf("inum : %d, reclen : %d, strlen : %d, name : %s\n", 
                curr_dir_entry.inum,
                curr_dir_entry.reclen,
                curr_dir_entry.strlen,
                curr_dir_entry.name
                );
            }
        }
    }
}


void add_file_to_directory(disk_mem *dm, uint16_t inode_number_of_dir, inode* file_inode, char* file_name)
{
    inode *curr_inode = dm->inode_list[inode_number_of_dir];
    dir_entry *needed_dir_entry = NULL;

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        dir *curr_dir_block = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
        for(int j = 0; j < (BLOCK_SIZE / sizeof(dir_entry)); j++)
        {
            if(curr_dir_block->data[j].inum == -1)
            {
                needed_dir_entry = &curr_dir_block->data[j];
                break;
            }
        }
        if(needed_dir_entry)
        {
            break;;
        }
    }

    needed_dir_entry->inum = file_inode->uid;
    needed_dir_entry->reclen = curr_inode->size / BLOCK_SIZE;
    needed_dir_entry->strlen = strlen(file_name);
    strcpy(needed_dir_entry->name, file_name);
}

uint16_t create_new_file(disk_mem *dm, uint32_t *file_data) // -1 - no free data blocks
{
    // find new free block, put there file data
    uint32_t new_data_index = -1;
    for(int i = 0; i < DATA_BLOCKS; i++)
    {
        if(NULL == dm->block_list[i])
        {
            new_data_index = i;
            break;
        }
    }
    //printf("here : %d\n", new_data_index);
    if(new_data_index == -1)
    {
        return -1;
    }

    block* new_data_block = (block*)malloc(sizeof(block));

    uint32_t size_of_data_block = sizeof(file_data) / sizeof(uint32_t);
    
    for(int i = 0; i < size_of_data_block; i++)
    {
        new_data_block->data[i] = file_data[i];
    }
    dm->block_list[new_data_index] = (block*)new_data_block;

    free(file_data);

    uint32_t inode_number_of_new_file;
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        if(NULL == dm->inode_list[i])
        {
            inode_number_of_new_file = i;
            break;
        }
    }

    inode *new_inode = (inode*)malloc(sizeof(inode));
    
    new_inode->mode = 0;
    new_inode->uid = inode_number_of_new_file;
    new_inode->size = sizeof(file_data);
    new_inode->mtime = get_curr_time();
    new_inode->links_count = 0;
    new_inode->direct_blocks = (uint32_t*)malloc(sizeof(uint32_t) * DIRECT_BLOCKS);
    new_inode->direct_blocks[0] = new_data_index;
    for(int i = 1; i < DIRECT_BLOCKS; i++)
    {
        new_inode->direct_blocks[i] = -1;
    } 
    new_inode->indirect_block = (uint32_t*)malloc(sizeof(uint32_t) * INDIRECT_BLOCKS);
    for(int i = 0; i < INDIRECT_BLOCKS; i++)
    {
        new_inode->indirect_block[i] = -1;
    }

    dm->inode_list[inode_number_of_new_file] = new_inode;
    return new_inode->uid;

}


uint16_t create_file(
    disk_mem* dm, 
    uint32_t inode_number_of_dir,
    char *file_name,
    uint32_t *file_data
    ) // -1 = file has not been created
{
    uint16_t new_inode_number = create_new_file(dm, file_data);

    inode *inode_of_new_file = NULL;
    for(int i = 0; i < INDIRECT_BLOCKS; i++)
    {
        if(dm->inode_list[i]->uid == new_inode_number)
        {
            inode_of_new_file = dm->inode_list[i];
            break;
        }
    }
    printf("here : %d : %d\n", inode_number_of_dir, inode_of_new_file->uid);
    if(new_inode_number != -1)
    {
        printf("here : %d : %d\n", inode_number_of_dir, inode_of_new_file->uid);
        add_file_to_directory(dm, inode_number_of_dir, inode_of_new_file, file_name);
        return new_inode_number;
    }
    return -1;
    
}


