#ifndef FILE_H
#define FILE_H

#include "file.h"  
#include "fs.h"
#include "utils.h"
#include "print.h"
#include "dir.h"

int8_t print_whole_data_from_file_by_file_name(disk_mem *dm, uint32_t inode_number_of_dir, char *file_name);
uint16_t create_new_file(disk_mem *dm);
uint16_t create_file(disk_mem* dm, uint32_t inode_number_of_dir, char *file_name);
int8_t add_data_to_file(disk_mem *dm, uint16_t file_inode_number, char *file_data, uint32_t data_size);
int8_t add_data_to_file_by_dir(disk_mem *dm, uint16_t directory_inode_number, char *file_name, char *file_data, uint32_t data_size, uint8_t flag);
int16_t delete_file_from_dir(disk_mem* dm, uint32_t file_inode, uint32_t dir_inode);

#endif 