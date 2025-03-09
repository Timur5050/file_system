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

    char file_name[30] = "hello.txt";
    uint32_t *data = (uint32_t*)malloc(sizeof(uint32_t) * 10);
    for(uint32_t i = 0; i < 10; i++)
    {   
        *(data + i) = i;
    }  

    int res = create_file(
        dm,
        0,
        file_name,
        data
    );
    printf("\n\n added file test : \n\n");
    print_all_inodes(dm);
    printf("\n\n");
    print_all_blocks(dm);
    printf("\n\n");

    print_all_dir_entries(dm, 0);

    return 0;
}
