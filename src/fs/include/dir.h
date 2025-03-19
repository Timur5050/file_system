#ifndef DIR_H
#define DIR_H

#include "dir.h"  
#include "fs.h"
#include "utils.h"
#include "file.h"

int8_t check_if_name_is_present_in_dir(disk_mem *dm, uint16_t dir_inode, char *new_name);
int32_t add_smth_to_dir(disk_mem *dm, uint16_t new_inode, uint16_t dir_inode, char *new_name);
int32_t create_new_dir_inode(disk_mem *dm);
int32_t create_dir(disk_mem *dm, uint32_t inode_number_of_dir, char *dir_name);
int16_t delete_dir_from_dir(disk_mem *dm, uint32_t dir_inode_to_del, uint32_t curr_dir_inode_index, int force);
int16_t delete_smth_by_name(disk_mem* dm, char* smth_name, uint32_t dir_inode, uint8_t mode_to_del, int force);


#endif 