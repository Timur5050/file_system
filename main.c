#include <stdio.h>
#include "fs.h"



uint32_t curr_dir_inode = 2;


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
    char *data = (char*)malloc(5000);
    for (int i = 0; i < 5000; i++) {
        data[i] = 'A' + (i % 26);
    }
    // int res = create_file(
    //     dm,
    //     0,
    //     file_name
    // );
    // printf("\n\n added file test : \n\n");
    // print_whole_dm(dm);
    // print_all_dir_entries(dm, 0);
    // printf("\n\n\n");

    // int8_t a = add_data_to_file(dm, 1, data, 10);
    // print_whole_data_from_file(dm, 1);
    //add_data_to_file(dm, 1, data, 2000);
    // printf("\n\n");
    // print_whole_data_from_file(dm, 1);
    // add_data_to_file(dm, 1, data, 10);
    // printf("\n\n");
    // print_whole_data_from_file(dm, 1);
    // int res1 = add_data_to_file_by_dir(dm, 0, "hello.txt", data, 10);
    // printf("\n\n");
    //print_whole_data_from_file(dm, 1);
    // printf("\n\nres : %d", res1);

    int a = create_dir(dm, 0, "new");
    printf("\n\n added dir test : \n\n");
    print_whole_dm(dm);
    print_all_dir_entries(dm, 0);
    // printf("\nres : %d\n", a);
    int res = create_file(
        dm,
        1,
        file_name
    );
    add_data_to_file(dm, 2, data, 5000);
    // print_whole_data_from_file(dm, 2);
    // printf("\n");
    // printf("\n\n added dir test : \n\n");
    // print_whole_dm(dm);

    // delete_smth_by_name(dm, "hello.txt", 1);

    // printf("\n\n added dir test : \n\n");
    // print_whole_dm(dm);

    // delete_smth_by_name(dm, "new", 0);

    // printf("\n\n added dir test : \n\n");
    // print_whole_dm(dm);

    // print_whole_dm(dm);

    int curr_inode = 1;
    char text[1000] = "";
    pwd(dm, curr_inode, text);

    printf("\n%s\n", text);
    ls(dm, curr_inode);

    // mkdir(dm, curr_inode, "hello1");
    // ls(dm, curr_inode);

    //touch(dm, curr_inode, "hello");
    //ls(dm, curr_inode);

    //print_whole_dm(dm);

    // rmdir(dm, curr_inode, "new");
    // ls(dm, curr_inode);
    cat(dm, 1, "hello.txt");

    printf("\n\n\n");

    echo(dm, 1, "hello.txt", 0);
    printf("\n data :");
    cat(dm, 1, "hello.txt");
    return 0;
}
