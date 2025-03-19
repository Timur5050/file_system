#include "cli/include/cli.h"


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
                    if(rmdir(dm, curr_dir_inode, braken_command[2], 1) == -5)
                    {
                        printf("rm: refusing to remove '.' or '..' directory: skipping '%s'\n", braken_command[2]);
                    }
                    else if(rm(dm, curr_dir_inode, braken_command[2]) < 0 
                    && rmdir(dm, curr_dir_inode, braken_command[2], 1) < 0)
                    {
                        printf("rm: cannot remove '%s': No such file or directory\n", braken_command[2]);
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
                if(res == -5)
                {
                    printf("rm: refusing to remove '.' or '..' directory: skipping '%s'\n", braken_command[2]);
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


int main() 
{
    start();
}
