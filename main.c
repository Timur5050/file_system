#include <stdio.h>
#include "fs.h"


typedef struct dir1 {
    int *a;
} dir1;

int main() {
    superblock sb;
    init_superblock(&sb);

    //printf("Total blocks: %u\n", sb.total_blocks);
    //printf("Inode start: %u\n", sb.inode_start);
    //printf("Data start: %u\n", sb.data_start);
    
    disk_mem* dm = (disk_mem*)malloc(sizeof(disk_mem));
    init_disk_mem(dm);
    init_root_directory(dm);
    print_all_inodes(dm);
    printf("\n\n");
    print_all_blocks(dm);
    printf("\n\n");

    print_all_dir_entries(dm, 0);


    return 0;
}
