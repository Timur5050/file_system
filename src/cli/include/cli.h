#ifndef CLI_H
#define CLI_H

#include "cli.h"  
#include "general.h"


char* pwd(disk_mem* dm, uint32_t curr_dir_inode, char *text_res);
void ls(disk_mem *dm, int32_t curr_dir_inode);
int32_t cd(disk_mem *dm, uint32_t *curr_dir_inode, char *new_dir_name);
int32_t mkdir(disk_mem *dm, uint32_t curr_dir_inode, char *dir_name);
int32_t touch(disk_mem *dm, uint32_t curr_dir_inode, char *file_name);
int32_t rm(disk_mem *dm, uint32_t curr_dir_inode, char *file_name);
int32_t rmdir(disk_mem *dm, uint32_t curr_dir_inode, char *dir_name, int force);
int32_t cat(disk_mem *dm, uint32_t curr_dir_inode, char *file_name);
int32_t echo(disk_mem *dm, uint32_t curr_dir_inode, char *text, int text_size, char *file_name, int flag);

#endif 