#include <stdio.h>
#include "fs.h"



uint32_t curr_dir_inode = 0;

char **break_the_command(char *command, int size_of_command, int *size_of_words_mass)
{
    char **mass = (char **)malloc(sizeof(char *) * 16); 
    if (!mass) return NULL;

    *size_of_words_mass = 0;
    char word[128]; 
    int word_counter = 0;

    for (int i = 0; i < size_of_command; i++)
    {   
        if (command[i] != ' ' && command[i] != '\0')
        {
            if (word_counter < 127)
                word[word_counter++] = command[i];
        }
        else if (word_counter > 0) 
        {
            word[word_counter] = '\0';
            if (*size_of_words_mass < 16)
            {
                mass[*size_of_words_mass] = strdup(word); 
                if (!mass[*size_of_words_mass]) return NULL; 
                (*size_of_words_mass)++;
            }
            word_counter = 0;
        }
    }

    if (word_counter > 0 && *size_of_words_mass < 16)
    {
        word[word_counter] = '\0';
        mass[*size_of_words_mass] = strdup(word);
        if (!mass[*size_of_words_mass]) return NULL;
        (*size_of_words_mass)++;
    }

    mass[*size_of_words_mass] = NULL; 
    return mass;
}


int start()
{
    superblock sb;
    init_superblock(&sb);
    
    disk_mem* dm = (disk_mem*)malloc(sizeof(disk_mem));
    init_disk_mem(dm);
    init_root_directory(dm);

    while(1)
    {
        int size_of_command = 0;
        char command[1024];
        char a;
        char text_first[1000] = "";
        pwd(dm, curr_dir_inode, text_first);
        printf("\033[1;34m%s\033[0m", text_first);
        printf("$ : ");
        while((a = getchar()) != '\n' && size_of_command < sizeof(command) - 1)
        {
            command[size_of_command++] = a;
        }
        if(size_of_command == sizeof(command) - 1)
        {
            printf("too long mess\n");
            continue;
        }
        command[size_of_command] = '\0';
        int size_of_words_mass;
        char **braken_command = break_the_command(command, size_of_command, &size_of_words_mass);
        
        if(strcmp(braken_command[0], "pwd") == 0)
        {
            char text[1000] = "";
            pwd(dm, curr_dir_inode, text);
            printf("%s\n", text);
        }
        else if(strcmp(braken_command[0], "ls") == 0)
        {
            ls(dm, curr_dir_inode);
        }
        else if(strcmp(braken_command[0], "cd") == 0)
        {
            if(size_of_words_mass < 2)
            {
                printf("too few arguments\n");
            }
            else if(size_of_words_mass > 2)
            {
                printf("too much arguments\n");
            }
            else
            {
                int res = cd(dm, &curr_dir_inode, braken_command[1]);
                if(res == -1)
                {
                    printf("cd: %s: no such directory\n", braken_command[1]);
                }
            }
        }
        else if(strcmp(braken_command[0], "exit") == 0)
        {
            return 0;
        }
        else if(strcmp(braken_command[0], "mkdir") == 0)
        {
            if(size_of_words_mass == 1)
            {
                printf("mkdir: missing operand\n");
            }
            else
            {
                for(int i = 1; i < size_of_words_mass; i++)
                {
                    int res = create_dir(dm, curr_dir_inode, braken_command[i]);
                    if(res < 0)
                    {
                        printf("mkdir: cannot create directory '%s': file or directory exists\n", braken_command[i]);
                    }
                }
            }
        }
        else if(strcmp(braken_command[0], "touch") == 0)
        {
            if(size_of_words_mass == 1)
            {
                printf("touch: missing file operand\n");
            }
            else
            {
                for(int i = 1; i < size_of_words_mass; i++)
                {
                    int res = touch(dm, curr_dir_inode, braken_command[i]);
                    if(res < 0)
                    {
                        printf("touch: cannot create directory '%s': file or directory exists\n", braken_command[i]);
                    }
                }
            }

        }
        else if(strcmp(braken_command[0], "rm") == 0)
        {
            if(size_of_words_mass == 1)
            {
                printf("rm: missing operand\n");
            }
            else
            {
                if(strcmp(braken_command[1], "-r") == 0)
                {
                    if(rm(dm, curr_dir_inode, braken_command[2]) < 0 
                    && rmdir(dm, curr_dir_inode, braken_command[2], 1) < 0)
                    {
                        printf("rm: cannot remove '%s': No such file or directory", braken_command[2]);
                    }
                }
                else
                {
                    int res = rm(dm, curr_dir_inode, braken_command[1]);
                    if(res < 0)
                    {
                        printf("rm: cannot remove '%s': No such file\n", braken_command[1]);
                    }
                }
            }
        }
        else if(strcmp(braken_command[0], "rmdir") == 0)
        {
            
            if(size_of_words_mass == 1)
            {
                printf("rmdir: missing operand\n");
            }
            else
            {
                int res = rmdir(dm, curr_dir_inode, braken_command[1], 0);
                if(res == -2)
                {
                    printf("mdir: failed to remove '%s': Directory not empty\n", braken_command[1]);
                }
                else if (res < 0)
                {
                    printf("rmdir: failed to remove '%s': No such file or directory\n", braken_command[1]);
                }
            }
        }
        else if(strcmp(braken_command[0], "cat") == 0)
        {
            if(size_of_words_mass == 1)
            {
                printf("cat: missing operand\n");
            }
            else
            {
                int res = cat(dm, curr_dir_inode, braken_command[1]);
                if(res < 0)
                {
                    printf("cat: failed to read '%s': No such file\n", braken_command[1]);
                }
            }
        }
        else if(strcmp(braken_command[0], "echo") == 0)
        {
            if(size_of_words_mass == 1)
            {
                printf("\n");
            }
            else if(size_of_words_mass == 2)
            {
                printf("%s\n", braken_command[1]);
            }
            else if(size_of_words_mass == 4)
            {
                int counter = 0;
                char *ptr = braken_command[1];
                while(*ptr != '\0')
                {
                    ptr++;
                    counter++;
                }
                if(strcmp(braken_command[2], ">") == 0)
                {
                    int res = echo(dm, curr_dir_inode, braken_command[1], counter, braken_command[3], 1);
                    if(res < 0)
                    {
                        touch(dm, curr_dir_inode, braken_command[3]);
                        echo(dm, curr_dir_inode, braken_command[1], counter, braken_command[3], 1);
                    }
                }
                else if(strcmp(braken_command[2], ">>") == 0)
                {
                    int res = echo(dm, curr_dir_inode, braken_command[1], counter, braken_command[3], 0);
                    if(res < 0)
                    {
                        printf("echo: failed adding text to file '%s', No such file\n", braken_command[3]);
                    }
                }
                
            }
        }
        else
        {
            printf("command %s not found\n", command);
        }
    }

}


int main() {
    // superblock sb;
    // init_superblock(&sb);

    // //printf("Total blocks: %u\n", sb.total_blocks);
    // //printf("Inode start: %u\n", sb.inode_start);
    // //printf("Data start: %u\n", sb.data_start);
    
    // disk_mem* dm = (disk_mem*)malloc(sizeof(disk_mem));
    // init_disk_mem(dm);
    // init_root_directory(dm);
    // print_whole_dm(dm);

    // print_all_dir_entries(dm, 0);

    // char file_name[30] = "hello.txt";
    // char *data = (char*)malloc(5000);
    // for (int i = 0; i < 5000; i++) {
    //     data[i] = 'A' + (i % 26);
    // }
    // // int res = create_file(
    // //     dm,
    // //     0,
    // //     file_name
    // // );
    // // printf("\n\n added file test : \n\n");
    // // print_whole_dm(dm);
    // // print_all_dir_entries(dm, 0);
    // // printf("\n\n\n");

    // // int8_t a = add_data_to_file(dm, 1, data, 10);
    // // print_whole_data_from_file(dm, 1);
    // //add_data_to_file(dm, 1, data, 2000);
    // // printf("\n\n");
    // // print_whole_data_from_file(dm, 1);
    // // add_data_to_file(dm, 1, data, 10);
    // // printf("\n\n");
    // // print_whole_data_from_file(dm, 1);
    // // int res1 = add_data_to_file_by_dir(dm, 0, "hello.txt", data, 10);
    // // printf("\n\n");
    // //print_whole_data_from_file(dm, 1);
    // // printf("\n\nres : %d", res1);

    // int a = create_dir(dm, 0, "new");
    // printf("\n\n added dir test : \n\n");
    // print_whole_dm(dm);
    // print_all_dir_entries(dm, 0);
    // // printf("\nres : %d\n", a);
    // int res = create_file(
    //     dm,
    //     1,
    //     file_name
    // );
    // add_data_to_file(dm, 2, data, 5000);
    // // print_whole_data_from_file(dm, 2);
    // // printf("\n");
    // // printf("\n\n added dir test : \n\n");
    // // print_whole_dm(dm);

    // // delete_smth_by_name(dm, "hello.txt", 1);

    // // printf("\n\n added dir test : \n\n");
    // // print_whole_dm(dm);

    // // delete_smth_by_name(dm, "new", 0);

    // // printf("\n\n added dir test : \n\n");
    // // print_whole_dm(dm);

    // // print_whole_dm(dm);

    // int curr_inode = 1;
    // char text[1000] = "";
    // pwd(dm, curr_inode, text);

    // printf("\n%s\n", text);
    // ls(dm, curr_inode);

    // // mkdir(dm, curr_inode, "hello1");
    // // ls(dm, curr_inode);

    // //touch(dm, curr_inode, "hello");
    // //ls(dm, curr_inode);

    // //print_whole_dm(dm);

    // // rmdir(dm, curr_inode, "new");
    // // ls(dm, curr_inode);
    // cat(dm, 1, "hello.txt");

    // printf("\n\n\n");

    // // echo(dm, 1, "hello.txt", 0);
    // // printf("\n data :");
    // // cat(dm, 1, "hello.txt");


    // printf("\n\n\n");

    // int counter = 0;
    // char mass[1024];
    // char chr;
    // while((chr = getchar()) != '\n' && counter < sizeof(mass) - 1)
    // {
    //     mass[counter++] = chr;
    // }
    // if(counter == sizeof(mass) - 1)
    // {
    //     printf("too long mess\n");
    // }
    // mass[counter] = '\0';
    // printf("\n%s\n", mass);
    // break_the_commad(mass, counter);
    // return 0;
    start();
}
