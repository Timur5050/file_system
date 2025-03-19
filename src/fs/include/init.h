#ifndef INIT_H
#define INIT_H

#include "init.h"  
#include "fs.h"
#include "utils.h"

void init_superblock(superblock *sb);
void init_root_directory(disk_mem *dm);
void init_disk_mem(disk_mem* dm);

#endif 