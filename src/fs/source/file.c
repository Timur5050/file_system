#include "../include/file.h"



int8_t print_whole_data_from_file_by_file_name(disk_mem *dm, uint32_t inode_number_of_dir, char *file_name)
{
    inode *curr_dir_inode = dm->inode_list[inode_number_of_dir];
    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_dir_inode->direct_blocks[i] != -1)
        {
            dir *curr_dir = (dir*)dm->block_list[curr_dir_inode->direct_blocks[i]];
            for(int i = 0; i < BLOCK_SIZE / sizeof(dir_entry); i++)
            {
                dir_entry *curr_entry = &curr_dir->data[i];
                if(curr_entry->inum != -1 && strcmp(curr_entry->name, file_name) == 0 && dm->inode_list[curr_entry->inum]->mode == 0)
                {
                    print_whole_data_from_file(dm, curr_entry->inum);
                    return 0;
                }
            }
        }
    }
    return -1;
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
    char *file_data,
    uint32_t data_size
) // -1 = error file adding data to file (inode not found) -2 - error no free data blocks
{

    inode *curr_inode = dm->inode_list[file_inode_number];
    if(NULL == curr_inode || curr_inode->mode != 0)
    {
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

        //curr_inode->size += sizeof(block);

        data_block_pointer = 0;
    }
    else
    {
        data_block_pointer = (curr_inode->size % sizeof(block));
    }
    block *bl = dm->block_list[curr_inode->direct_blocks[last_block_index]];
    uint32_t new_data_index = 0;

    while(last_block_index < 8 && new_data_index < data_size)
    {
        while(data_block_pointer < BLOCK_SIZE && new_data_index < data_size)
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
                    break;
                }
            }
        }
    }
    
    curr_inode->size += data_size;
    
    return file_inode_number;
}

int8_t add_data_to_file_by_dir(
    disk_mem *dm,
    uint16_t directory_inode_number,
    char *file_name,
    char *file_data,
    uint32_t data_size,
    uint8_t flag
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
            if(strcmp(curr_dir_entry.name, file_name) == 0 && dm->inode_list[curr_dir_entry.inum]->mode == 0)
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
    
    if(flag == 1)
    {
        inode *file_inode_curr = dm->inode_list[file_inode];
        file_inode_curr->size = 0;
        for(int i = 0; i < DIRECT_BLOCKS; i++)
        {
            if(file_inode_curr->direct_blocks[i] != -1)
            {
                free(dm->block_list[file_inode_curr->direct_blocks[i]]);
                file_inode_curr->direct_blocks[i] = -1;
            }
        }
    }


    return add_data_to_file(dm, file_inode, file_data, data_size);
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
