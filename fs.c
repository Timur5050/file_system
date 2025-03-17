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
    root_dir->data[0].reclen = BLOCK_SIZE / sizeof(dir_entry);
    root_dir->data[0].strlen = 1;
    strcpy(root_dir->data[0].name, ".");

    dm->inode_list[0] = root_inode;
}


void init_disk_mem(disk_mem* dm)
{
    superblock* sb = (superblock*)malloc(sizeof(superblock));
    init_superblock(sb);
    
    uint8_t* i_mp = (uint8_t*)calloc(BITMAP_INODE_BLOCKS, sizeof(uint8_t));
    uint8_t* d_mp = (uint8_t*)calloc(BITMAP_DATA_BLOCKS, sizeof(uint8_t));

    
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
    if(NULL == curr_dir) 
    {
        return 2;
    }

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
    printf("---- inode list ----\n");

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
        printf("\n\n");
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
        for(int j = 0; j < INODE_BLOCKS; j++)
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


void print_i_bmap_and_d_bmap(disk_mem *dm)
{
    printf("---- bit masks list ----\n");

    printf("inode bit map : ");
    for(int i = 0; i < BITMAP_INODE_BLOCKS; i++)
    {
        printf("%d ", dm->i_bmap[i]);
    }
    printf("\ndata bit map : ");
    for(int i = 0; i < BITMAP_DATA_BLOCKS; i++)
    {
        printf("%d ", dm->d_bmap[i]);
    }
    printf("\n");
}


void print_whole_dm(disk_mem *dm)
{
    print_all_inodes(dm);
    printf("\n\n");
    print_all_blocks(dm);
    printf("\n\n");
    print_i_bmap_and_d_bmap(dm);
    printf("\n\n");
}

void print_whole_data_from_file(disk_mem *dm, uint32_t inode_number)
{
    inode *curr_inode = dm->inode_list[inode_number];
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            block *curr_block = dm->block_list[curr_inode->direct_blocks[i]];
            uint16_t size = sizeof(curr_block->data) / sizeof(uint32_t);
            for(int j = 0; j < size; j++)
            {
                printf("%d ", curr_block->data[j]);
            }            
        }
    }
}


uint16_t create_new_file(disk_mem *dm) // -1 - no free data blocks
{
    uint32_t inode_number_of_new_file;
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        if(NULL == dm->inode_list[i])
        {
            inode_number_of_new_file = i;
            break;
        }
    }

    if(inode_number_of_new_file == -1)
    {
        return -1;
    }

    dm->i_bmap[inode_number_of_new_file] = 1;

    inode *new_inode = (inode*)malloc(sizeof(inode));
    
    new_inode->mode = 0;
    new_inode->uid = inode_number_of_new_file;
    new_inode->size = 0;
    new_inode->mtime = get_curr_time();
    new_inode->links_count = 1;
    new_inode->direct_blocks = (uint32_t*)malloc(sizeof(uint32_t) * DIRECT_BLOCKS);

    for(int i = 0; i < DIRECT_BLOCKS; i++)
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
    char *file_name
    ) // -1 = file has not been created
{
    uint16_t new_inode_number = create_new_file(dm);

    inode *inode_of_new_file = NULL;
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        if(dm->inode_list[i] != NULL && dm->inode_list[i]->uid == new_inode_number)
        {
            inode_of_new_file = dm->inode_list[i];
            break;
        }
    }
    if(new_inode_number != -1)
    {
        add_smth_to_dir(dm, inode_of_new_file->uid, inode_number_of_dir, file_name);
        return new_inode_number;
    }
    return -1;
    
}


int8_t add_data_to_file(
    disk_mem *dm, 
    uint16_t file_inode_number, 
    uint32_t *file_data,
    uint32_t data_size
) // -1 = error file adding data to file (inode not found) -2 - error no free data blocks
{

    inode *curr_inode = dm->inode_list[file_inode_number];
    if(NULL == curr_inode || curr_inode->mode != 0)
    {
        printf("\nhere -1 : %d\n", file_inode_number);
        return -1;
    }

    int last_block_index = -1;
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] == -1)
        {
            break;
        }
        last_block_index = i;
    }

    uint32_t data_block_pointer;

    if(last_block_index == -1)
    {
        last_block_index = 0;
        for(int i = 0; i < DATA_BLOCKS; i++)
        {
            if(NULL == dm->block_list[i] && dm->i_bmap[i] == 0)
            {
                curr_inode->direct_blocks[0] = i;
                break;
            }
            
        }
        if(curr_inode->direct_blocks[0] == -1)
        {
            return -2;
        }

        block *new_block = (block*)malloc(sizeof(block));
        dm->block_list[curr_inode->direct_blocks[0]] = new_block;
        dm->d_bmap[curr_inode->direct_blocks[0]] = 1;

        curr_inode->size += sizeof(block);

        data_block_pointer = 0;
    }
    else
    {
        data_block_pointer = (curr_inode->size % sizeof(block)) / sizeof(uint32_t);
    }
    block *bl = dm->block_list[curr_inode->direct_blocks[last_block_index]];
    uint32_t new_data_index = 0;

    while(last_block_index < 8 && new_data_index < data_size)
    {
        while(data_block_pointer < BLOCK_SIZE / sizeof(uint32_t) && new_data_index < data_size)
        {
            bl->data[data_block_pointer] = file_data[new_data_index];
            new_data_index ++;
            data_block_pointer ++;
        }
        if(new_data_index < data_size && last_block_index + 1 < DIRECT_BLOCKS)
        {
            last_block_index ++;
            for(int i = 0; i < DATA_BLOCKS; i++)
            {
                if(dm->block_list[i] == NULL && dm->d_bmap[i] == 0)
                {
                    curr_inode->direct_blocks[last_block_index] = i;
                    dm->block_list[i] = (block*)malloc(sizeof(block));
                    if (!dm->block_list[i]) return -2;

                    memset(dm->block_list[i], 0, sizeof(block));
                    dm->d_bmap[i] = 1;

                    bl = dm->block_list[i];
                    data_block_pointer = 0;  
                    curr_inode->size += sizeof(block);
                    break;
                }
            }
        }
    }
    // curr_inode->size += sizeof(uint32_t) * data_size;
    return file_inode_number;
}

int8_t add_data_to_file_by_dir(
    disk_mem *dm,
    uint16_t directory_inode_number,
    char *file_name,
    uint32_t *file_data,
    uint32_t data_size
) // -1 - no such dir, -2  - no such file
{
    inode *curr_inode = dm->inode_list[directory_inode_number];
    if(NULL == curr_inode)
    {
        return -1;
    }

    uint32_t file_inode = -1;

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] == -1)
        {
            continue;
        }
        dir *dir_block = dm->block_list[curr_inode->direct_blocks[i]];
        for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
        {
            dir_entry curr_dir_entry = dir_block->data[j];
            if(strcmp(curr_dir_entry.name, file_name) == 0)
            {
                file_inode = curr_dir_entry.inum;
                break;
            }
        }
    }

    if(file_inode == -1)
    {
        return -2;
    }
    
    return add_data_to_file(dm, file_inode, file_data, data_size);
}


int32_t create_new_dir_inode(disk_mem *dm) // -1 = no free inode, -2 = no free block
{
    uint16_t new_inode_index = -1;

    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        if(NULL == dm->inode_list[i])
        {
            new_inode_index = i;
            break;
        }
    }
    if(new_inode_index == -1)
    {
        return -1;
    }

    dm->i_bmap[new_inode_index] = 1;

    inode *new_inode = (inode*)malloc(sizeof(inode));
    new_inode->mode = 1;
    new_inode->uid = new_inode_index;
    new_inode->size = 0;
    new_inode->mtime = get_curr_time();
    new_inode->links_count = 1;
    new_inode->direct_blocks = (uint32_t*)malloc(sizeof(uint32_t) * DIRECT_BLOCKS);
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        new_inode->direct_blocks[i] = -1;
    }   

    new_inode->indirect_block = (uint32_t*)malloc(sizeof(uint32_t) * INODE_BLOCKS);
    for(int i = 0; i < INODE_BLOCKS; i++)
    {
        new_inode->indirect_block[i] = -1;
    }
    dm->inode_list[new_inode_index] = new_inode;
    
    return new_inode_index;
}


int32_t create_dir(
    disk_mem *dm,
    uint32_t inode_number_of_dir,
    char *dir_name
) 
{
    if(check_if_name_is_present_in_dir(dm, inode_number_of_dir, dir_name) == -1)
    {
        return -4;
    }

    inode* parent_dir_inode = dm->inode_list[inode_number_of_dir];
    if (NULL == parent_dir_inode) 
    {
        return -3;
    }

    uint16_t new_inode_dir = create_new_dir_inode(dm);
    if (new_inode_dir < 0) 
    {
        return -1;
    }


    int32_t result = add_smth_to_dir(dm, new_inode_dir, inode_number_of_dir, dir_name);
    if (result < 0) 
    {
        return result; 
    }


    inode *new_dir_inode = dm->inode_list[new_inode_dir];


    int32_t new_dir_block_index = -1;
    for (int i = 0; i < DATA_BLOCKS; i++) 
    {
        if (NULL == dm->block_list[i]) 
        {
            new_dir_block_index = i;
            break;
        }
    }

    if (new_dir_block_index == -1) 
    {
        return -2; 
    }

    dir *new_dir_block = (dir*)malloc(sizeof(dir));
    if (!new_dir_block) 
    {
        return -2; 
    }

    for (int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++) 
    {
        new_dir_block->data[i].inum = -1;
    }

    new_dir_block->data[0].inum = new_inode_dir;
    new_dir_block->data[0].reclen = 1;
    new_dir_block->data[0].strlen = 1;
    strcpy(new_dir_block->data[0].name, ".");

    new_dir_block->data[1].inum = inode_number_of_dir;
    new_dir_block->data[1].reclen = 1;
    new_dir_block->data[1].strlen = 2;
    strcpy(new_dir_block->data[1].name, "..");

    new_dir_inode->direct_blocks[0] = new_dir_block_index;
    dm->block_list[new_dir_block_index] = new_dir_block;
    dm->d_bmap[new_dir_block_index] = 1;

    return new_inode_dir;
}


int8_t check_if_name_is_present_in_dir(
    disk_mem *dm, 
    uint16_t dir_inode,
    char *new_name
)
{
    inode *curr_inode = dm->inode_list[dir_inode];

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            dir *temp_dir = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry *temp_entry = &temp_dir->data[j];
                if(strcmp(temp_entry->name, new_name) == 0)
                {
                    return -1;
                }
            }
        }
    }
    return 0;
}


int32_t add_smth_to_dir(
    disk_mem *dm, 
    uint16_t new_inode,
    uint16_t dir_inode,
    char *new_name
) // -1 = no such file by inode; -2 = no such dir by inode; -3 = no free space in a dir
{

    if(check_if_name_is_present_in_dir(dm, dir_inode, new_name) == -1)
    {
        return -4;
    }

    if(NULL == dm->inode_list[new_inode])
    {
        return -1;
    }
    
    inode *curr_dir_inode = NULL;
    if(NULL == dm->inode_list[dir_inode])
    {
        return -2;
    }
    curr_dir_inode = dm->inode_list[dir_inode];

    dir* curr_dir = NULL;
    for(int i = 0; i < DIRECT_BLOCKS; i++) 
    {
        if(curr_dir_inode->direct_blocks[i] == -1)
        {
            continue;
        }
        curr_dir = (dir*)dm->block_list[curr_dir_inode->direct_blocks[i]];
        if(curr_dir != NULL) break;
    }
    if(NULL == curr_dir)
    {
        return -3;
    }
    dir_entry *new_dir_entry = NULL;
    for(int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        if(curr_dir->data[i].inum == -1)
        {
            new_dir_entry = &curr_dir->data[i];
            break;
        }
    }
    
    if(NULL == new_dir_entry)
    {
        for(int i = 0; i < DIRECT_BLOCKS; i++)
        {
            if(curr_dir_inode->direct_blocks[i] == -1)
            {
                for(int j = 0; j < DATA_BLOCKS; j++)
                {
                    if(NULL == dm->block_list[j])
                    {
                        curr_dir_inode->direct_blocks[i] = j;
                        dm->d_bmap[j] = 1;
                        dir* new_dir = (dir*)malloc(sizeof(dir));
                        dm->block_list[j] = new_dir;
                        

                        for(int k = 1; k < BLOCK_SIZE / sizeof(dir_entry); k++)
                        {
                            new_dir->data[k].inum = -1;
                        }

                        new_dir_entry = &new_dir->data[0];
                        break;
                    }
                }
                break;
            }
        }
    }

    if(NULL == new_dir_entry)
    {
        return -3;
    }

    new_dir_entry->inum = new_inode;
    int counter = 0;
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(dm->inode_list[new_inode]->direct_blocks[i] != -1)
        {
            counter += 1;
        }
    }
    if(dm->inode_list[new_inode]->mode == 0)
    {
        new_dir_entry->reclen = counter;
    }
    else if(dm->inode_list[new_inode]->mode == 1)
    {
        new_dir_entry->reclen = counter * (BLOCK_SIZE / sizeof(dir_entry));
    }
    new_dir_entry->strlen = strlen(new_name);
    strcpy(new_dir_entry->name, new_name);

    return curr_dir_inode->uid;

}

int16_t delete_file_from_dir(disk_mem* dm, uint32_t file_inode, uint32_t dir_inode)
{
    inode *inode_of_deleting_item = dm->inode_list[file_inode];
    if (!inode_of_deleting_item) 
    {
        return -1;
    }

    inode *curr_dir_inode = dm->inode_list[dir_inode];
    if (!curr_dir_inode)
    {
        return -1;
    } 


    for (int i = 0; i < DIRECT_BLOCKS; i++) 
    {
        if (curr_dir_inode->direct_blocks[i] != -1)
        {
            dir *curr_dir = (dir *)dm->block_list[curr_dir_inode->direct_blocks[i]];
            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                if (curr_dir->data[j].inum == file_inode)
                {
                    curr_dir->data[j].inum = -1; 
                }
            }
        }
    }

    if(inode_of_deleting_item->links_count == 1)
    {
        for(int i = 0; i < DIRECT_BLOCKS; i++)
        {
            if(inode_of_deleting_item->direct_blocks[i] != -1 && dm->block_list[inode_of_deleting_item->direct_blocks[i]] != NULL)
            {
                free(dm->block_list[inode_of_deleting_item->direct_blocks[i]]);
                dm->block_list[inode_of_deleting_item->direct_blocks[i]] = NULL;
                dm->d_bmap[inode_of_deleting_item->direct_blocks[i]] = 0;
            }
        }
        if (inode_of_deleting_item != NULL) 
        {
            free(inode_of_deleting_item);
            dm->inode_list[file_inode] = NULL;
            dm->i_bmap[file_inode] = 0;
            inode_of_deleting_item = NULL; 
        }
    }
    else
    {
        inode_of_deleting_item->links_count--;
    }
    return 0;
}

int16_t delete_dir_from_dir(disk_mem *dm, uint32_t dir_inode_to_del, uint32_t curr_dir_inode_index, int force)
{
    if (!dm || dir_inode_to_del >= INODE_BLOCKS || curr_dir_inode_index >= INODE_BLOCKS) 
    {
        return -1; 
    }

    inode *inode_of_deleting_item = dm->inode_list[dir_inode_to_del];
    if (!inode_of_deleting_item || inode_of_deleting_item->mode != 1) 
    {
        return -1;
    }

    inode *curr_dir_inode = dm->inode_list[curr_dir_inode_index];
    if (!curr_dir_inode) 
    {
        return -1;
    }

    dir_entry *entry_to_del = NULL;
    

    for (int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if (curr_dir_inode->direct_blocks[i] != -1)
        {
            dir *curr_dir = dm->block_list[curr_dir_inode->direct_blocks[i]];
            if (!curr_dir) continue;

            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                if (curr_dir->data[j].inum == dir_inode_to_del)
                {
                    entry_to_del = &curr_dir->data[j];
                }
            }
        }
    }

    int is_empty = 1; 


    for (int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if (inode_of_deleting_item->direct_blocks[i] != -1)
        {
            dir *curr_dir = dm->block_list[inode_of_deleting_item->direct_blocks[i]];
            if (!curr_dir) continue;

            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                if (curr_dir->data[j].inum != -1 &&
                    strcmp(curr_dir->data[j].name, ".") != 0 &&
                    strcmp(curr_dir->data[j].name, "..") != 0)
                {
                    is_empty = 0; 
                    if(force == 0)
                    {
                        return -2;
                    }
                    inode *child_inode = dm->inode_list[curr_dir->data[j].inum];

                    if (!child_inode) continue;

                    if (child_inode->mode == 0) 
                    {
                        delete_file_from_dir(dm, curr_dir->data[j].inum, dir_inode_to_del);
                    }
                    else if (child_inode->mode == 1)
                    {
                        int status = delete_dir_from_dir(dm, curr_dir->data[j].inum, dir_inode_to_del, force);
                        if (status != 0) 
                        {
                            return status; 
                        }
                    }
                }
            }
        }
    }

    if (!is_empty && !force) 
    {
        return -2; 
    }
    for (int i = 0; i < DIRECT_BLOCKS; i++) 
    {
        if (inode_of_deleting_item->direct_blocks[i] != -1) 
        {
            if (dm->block_list[inode_of_deleting_item->direct_blocks[i]] != NULL) 
            {
                free(dm->block_list[inode_of_deleting_item->direct_blocks[i]]);
                dm->block_list[inode_of_deleting_item->direct_blocks[i]] = NULL;
                dm->d_bmap[inode_of_deleting_item->direct_blocks[i]] = 0;
            }
        }
    }

    if (dm->inode_list[dir_inode_to_del] != NULL) 
    {
        free(inode_of_deleting_item);
        dm->inode_list[dir_inode_to_del] = NULL;
        dm->i_bmap[dir_inode_to_del] = 0;
    }

    if (entry_to_del != NULL)
    {
        entry_to_del->inum = -1;
        memset(entry_to_del->name, 0, sizeof(entry_to_del->name));
    }

    return 0; 
}



int16_t delete_smth_by_name(disk_mem* dm, char* smth_name, uint32_t dir_inode, uint8_t mode_to_del, int force)
{
    inode *curr_inode = dm->inode_list[dir_inode];
    if(!curr_inode)
    {
        return -1;
    }   
    
    int res = 1;
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            dir *curr_dir = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry *curr_dir_entr = &curr_dir->data[j];
                if(curr_dir_entr->inum != -1 && strcmp(curr_dir_entr->name, smth_name) == 0)
                {
                    inode *entry_inode = dm->inode_list[curr_dir_entr->inum];
                    if (!entry_inode) continue; 
                    if(mode_to_del != entry_inode->mode)
                    {
                        return -1;
                    }
                    
                    if(entry_inode->mode == 0)
                    {
                        res = delete_file_from_dir(dm, entry_inode->uid, dir_inode);
                    }
                    else if(entry_inode->mode == 1)
                    {
                        if(force)
                        {
                            res = delete_dir_from_dir(dm, entry_inode->uid, dir_inode, 1);
                        }
                        else
                        {
                            res = delete_dir_from_dir(dm, entry_inode->uid, dir_inode, 0);
                        }
                        
                    }
                    
                    if(res == 0)
                    {
                        curr_dir_entr->inum = -1;
                        memset(curr_dir_entr->name, 0, sizeof(curr_dir_entr->name));
                    }
                    return 0;
                }
            }
        }
    }
    return -1;
}

char* pwd(disk_mem* dm, uint32_t curr_dir_inode, char *text_res)
{
    if (curr_dir_inode == 0)
    {
        if(strlen(text_res) == 0)
        {
            strcat(text_res, "/");
            return text_res;
        }
        
        return text_res;
    }

    char result[1000] = ""; 
    char temp[300] = "";     

    inode *curr_inode = dm->inode_list[curr_dir_inode];
    inode *parent_dir_inode = NULL;

    for (int i = 0; i < DIRECT_BLOCKS; i++)
    {   
        if (curr_inode->direct_blocks[i] != -1)
        {
            dir *temp_dir = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
            for (int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry *temp_dir_entry = &temp_dir->data[j];

                if (strcmp(temp_dir_entry->name, "..") == 0) 
                {
                    parent_dir_inode = dm->inode_list[temp_dir_entry->inum]; 

                    for (int k = 0; k < DIRECT_BLOCKS; k++)
                    {
                        if (parent_dir_inode->direct_blocks[k] != -1)
                        {
                            dir *temp_dir_parent = (dir*)dm->block_list[parent_dir_inode->direct_blocks[k]];
                            for (int q = 0; q < BLOCK_SIZE / sizeof(dir_entry); q++)
                            {
                                dir_entry *temp_dir_parent_entry = &temp_dir_parent->data[q];

                                if (temp_dir_parent_entry->inum == curr_dir_inode)
                                {
                                    snprintf(temp, sizeof(temp), "/%s", temp_dir_parent_entry->name);
                                    strcat(temp, text_res);
                                    strcpy(text_res, temp); 
                                    return pwd(dm, temp_dir_entry->inum, text_res);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return text_res;
}

void ls(disk_mem *dm, int32_t curr_dir_inode)
{
    inode *curr_inode = dm->inode_list[curr_dir_inode];

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            dir *temp_dir = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry *temp_entry = &temp_dir->data[j];
                if(temp_entry->inum != -1)
                {
                    printf("%s  ", temp_entry->name);
                }
            }
        }
    }
    printf("\n");
}

int32_t cd(disk_mem *dm, uint32_t *curr_dir_inode, char *new_dir_name)
{
    inode *curr_inode = dm->inode_list[*curr_dir_inode];

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            dir *temp_dir = (dir*)dm->block_list[curr_inode->direct_blocks[i]];
            for(int j = 0; j < BLOCK_SIZE / sizeof(dir_entry); j++)
            {
                dir_entry *temp_entry = &temp_dir->data[j];
                if(temp_entry->inum != -1)
                {
                    if(strcmp(temp_entry->name, new_dir_name) == 0)
                    {
                        *curr_dir_inode = temp_entry->inum;
                        return *curr_dir_inode;
                    }
                }
            }
        }
    }
    return -1;

}

int32_t mkdir(disk_mem *dm, uint32_t curr_dir_inode, char *dir_name)
{
    return create_dir(dm, curr_dir_inode, dir_name);
}

int32_t touch(disk_mem *dm, uint32_t curr_dir_inode, char *file_name)
{
    return create_file(dm, curr_dir_inode, file_name);
}

int32_t rm(disk_mem *dm, uint32_t curr_dir_inode, char *file_name)
{
    return delete_smth_by_name(dm, file_name, curr_dir_inode, 0, 1);
}

int32_t rmdir(disk_mem *dm, uint32_t curr_dir_inode, char *dir_name)
{
    return delete_smth_by_name(dm, dir_name, curr_dir_inode, 1, 0);
}
