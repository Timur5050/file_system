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
    print_whole_dm(dm);

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
    print_whole_dm(dm);
    print_all_dir_entries(dm, 0);
    printf("\n\n\n");
    // block* temp = (block*)malloc(sizeof(block));
    // temp->data[0] = 100;
    // temp->data[1] = 2;
    // dm->block_list[1] = temp;
    // dm->inode_list[1]->direct_blocks[0] = 1;

    int8_t a = add_data_to_file(dm, 1, data, 10);
    print_whole_data_from_file(dm, 1);
    add_data_to_file(dm, 1, data, 10);
    printf("\n\n");
    print_whole_data_from_file(dm, 1);
    add_data_to_file(dm, 1, data, 10);
    printf("\n\n");
    print_whole_data_from_file(dm, 1);
    int res1 = add_data_to_file_by_dir(dm, 0, "hello.txt", data, 10);
    printf("\n\n");
    print_whole_data_from_file(dm, 1);
    printf("\n\nres : %d", res1);
    return 0;
}
