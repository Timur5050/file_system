#include "../include/dir.h"



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
    if(strcmp(smth_name, ".") == 0 || strcmp(smth_name, "..") == 0)
    {
        return -5;
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
                        return 0;
                    }
                    if(res == -2)
                    {
                        return -2;
                    }
                }
            }
        }
    }
    return -1;
}