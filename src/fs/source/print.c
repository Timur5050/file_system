#include "../include/print.h"

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
    uint16_t size = curr_inode->size;
    uint16_t start = 0;

    for(int i = 0; i < DIRECT_BLOCKS; i++)
    {
        if(curr_inode->direct_blocks[i] != -1)
        {
            block *curr_block = dm->block_list[curr_inode->direct_blocks[i]];
            for(int j = 0; j < BLOCK_SIZE; j++)
            {
                printf("%c", curr_block->data[j]);
                start++;
                if(start == size)
                {
                    printf("\n");
                    return;
                }
            }            
        }
    }
}
