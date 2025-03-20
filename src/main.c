#include "cli/include/cli.h"


uint32_t curr_dir_inode = 0;

void convert_command_text(char *text, char* res_mass)
{
    char **mass = (char**)malloc(sizeof(char*) * 256);
    int mass_counter = 0;
    char *ptr = text;
    char temp_mass[2048];
    int temp_counter = 0;
    int bracket_counter = 0;
    while(*ptr != '\0')
    {
        if(*ptr == '"')
        {
            if(bracket_counter % 2 != 0)
            {
                if(temp_counter > 0)
                {
                    temp_mass[temp_counter] = '\0';
                    mass[mass_counter] = strdup(temp_mass);
                    mass_counter++;
                    temp_counter = 0;
                }
            }
            bracket_counter++;
        }
        else if(*ptr != ' ' && *ptr != '"')
        {
            temp_mass[temp_counter] = *ptr;
            temp_counter++;
        }
        else if(*ptr == ' ' && bracket_counter % 2 != 0)
        {
            temp_mass[temp_counter] = *ptr;
            temp_counter++;
        }
        else if(*ptr == ' ' && bracket_counter % 2 == 0)
        {
            if(temp_counter > 0)
            {
                temp_mass[temp_counter] = '\0';
                mass[mass_counter] = strdup(temp_mass);
                mass_counter++;
                temp_counter = 0;
            }
        }
        ptr++;
    }   
    if(temp_counter > 0)
    {
        temp_mass[temp_counter] = '\0';
        mass[mass_counter++] = strdup(temp_mass);
    }
    int counter = 0;
    for (int i = 0; i < mass_counter; i++)
    {
        char *word = mass[i];
        while (*word != '\0')
        {
            res_mass[counter++] = *word++;
        }
        if (i < mass_counter - 1)
        {
            res_mass[counter++] = ' '; 
        }
        free(mass[i]); 
    }
    res_mass[counter] = '\0'; 

    free(mass); 
}


char **break_echo_longer_command(char *command, int i, int *size_of_words_mass)
{
    char **mass = (char **)malloc(sizeof(char *) * 16); 
    mass[0] = strdup("echo");
    *size_of_words_mass = 0;
    *size_of_words_mass += 1;
    char* ptr = command + i;
    
    int brackets_counter = 0;
    char text_to_out[8192];
    memset(text_to_out, 0, sizeof(text_to_out));
    int text_counter = 0;
    char a = 0;
    while(1)
    {
        if((brackets_counter % 2 == 0 && *ptr == '>') || *ptr == '\0')
        {
            break;
        }   
        if(*ptr == '"')
        {
            brackets_counter++;
        }
        text_to_out[text_counter] = *ptr;
        text_counter++;
        ptr++;
    }
    if(*ptr == '>')
    {
        char *text_to_add = (char*)malloc(4096);
        convert_command_text(text_to_out, text_to_add);
        mass[1] = strdup(text_to_add);
        ptr++;
        if(*ptr == '>')
        {
            mass[2] = strdup(">>");
        }
        else
        {
            mass[2] = strdup(">");
        }
        *size_of_words_mass += 2;
        
        char *temp_file_name = (char*)malloc(2048);
        int file_name_counter = 0;
        while(*ptr == ' ' || *ptr == '>') ptr++;
        while(*ptr != '\0' && *ptr != ' ')
        {
            temp_file_name[file_name_counter] = *ptr;
            ptr++;
            file_name_counter++;
        }
        temp_file_name[file_name_counter] = '\0';
        mass[3] = strdup(temp_file_name);
        *size_of_words_mass += 1;
        free(temp_file_name);
        free(text_to_add);
        return mass;
    }
    text_to_out[text_counter++] = '\n';
    while(1)
    {
        a = getchar();
        if(a == '"') brackets_counter++;
        if(brackets_counter % 2 == 0 && a == '>')
        {
            break;
        }
        else
        {
            text_to_out[text_counter] = a;
            text_counter++;
        }
    }
    text_to_out[text_counter] = '\0';
    char *text_to_add = (char*)malloc(4096);
    convert_command_text(text_to_out, text_to_add);
    mass[1] = strdup(text_to_add);

    a = getchar();

    if(a == '>')
    {
        mass[2] = strdup(">>");
    }
    else
    {
        mass[2] = strdup(">");
    }
    *size_of_words_mass += 2;
    char *temp_file_name = (char*)malloc(2048);
    int file_name_counter = 0;

    while(a == ' ' || a == '>') a = getchar(); 
    
    while(a != '\n')
    {
        temp_file_name[file_name_counter] = a;
        a = getchar();
        file_name_counter++;
    }
    temp_file_name[file_name_counter] = '\0';
    mass[3] = temp_file_name;
    *size_of_words_mass += 1;
    free(temp_file_name);
    free(text_to_add);
    return mass;
}


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

                if(strcmp(word, "echo") == 0)
                {
                    return break_echo_longer_command(command, i, size_of_words_mass);
                }

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
