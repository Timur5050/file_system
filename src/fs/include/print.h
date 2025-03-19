#ifndef PRINT_H
#define PRINT_H

#include "print.h"  
#include "fs.h"

void print_all_inodes(disk_mem* dm);
void print_all_blocks(disk_mem* dm);
void print_all_dir_entries(disk_mem* dm, uint32_t inode_number_of_dir);
void print_i_bmap_and_d_bmap(disk_mem *dm);
void print_whole_dm(disk_mem *dm);
void print_whole_data_from_file(disk_mem *dm, uint32_t inode_number);

#endif 