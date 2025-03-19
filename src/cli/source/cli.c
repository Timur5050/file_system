#include "../include/cli.h"



char* pwd(disk_mem* dm, uint32_t curr_dir_inode, char *text_res)
{
    if (curr_dir_inode == 0)
    {
        if(strlen(text_res) == 0)
        {
            strcat(text_res, "~");
            return text_res;
        }
        char text_temp[1000];
        char *ptr = text_res;
        int counter = 1;
        text_temp[0] = '~';
        while(*ptr != '\0')
        {
            text_temp[counter++] = *(ptr++);
        }
        text_temp[counter] = '\0';
        text_res[0] = '\0';
        strcpy(text_res, text_temp);
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
                    if(strcmp(temp_entry->name, new_dir_name) == 0 && dm->inode_list[temp_entry->inum]->mode == 1)
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

int32_t rmdir(disk_mem *dm, uint32_t curr_dir_inode, char *dir_name, int force)
{
    return delete_smth_by_name(dm, dir_name, curr_dir_inode, 1, force);
}

int32_t cat(disk_mem *dm, uint32_t curr_dir_inode, char *file_name)
{
    return print_whole_data_from_file_by_file_name(dm, curr_dir_inode, file_name);
}

int32_t echo(disk_mem *dm, uint32_t curr_dir_inode, char *text, int text_size, char *file_name, int flag)
{
    return add_data_to_file_by_dir(dm, curr_dir_inode, file_name, text, text_size, flag);
}
