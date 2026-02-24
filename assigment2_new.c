#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#define inode_buffer 4
#define char_buffer 32
#define input_buffer 100
#define ls_buffer (sizeof(uint32_t) + 32)


struct inode_file{
    
    // 1 or 0 if in use or not
    int in_use;

    // d or f type
    char f_type;

    // 0 - 1023
    int inode_num;
};


int inode_helper(int inode_num, struct inode_file inode_list[], char *target){
    
    char filename[16];
    uint32_t start_inode;
    
    // setting 32 bytes
    unsigned char namebuf[char_buffer];

    
    // check if inode num between 0-1023
    if (inode_num < 0 || inode_num > 1023){
        fprintf(stderr, "inode_helper: inode %d out of range\n", inode_num);
        return -1;
    }

    // check if inode in use
    if (inode_list[inode_num].in_use != 1) {
        fprintf(stderr, "inode_helper: inode %d is not in use\n", inode_num);
        return -1;
    }

    // check if its a directory
    if (inode_list[inode_num].f_type != 'd') {
        fprintf(stderr, "inode_helper: inode %d is not a directory\n", inode_num);
        return -1;
    }

    // inode number to string
    sprintf(filename, "%d", inode_num); 

    //open inode file 
    FILE *file_pointer = fopen(filename, "rb");  

    if (file_pointer == NULL) {
        perror("inode_helper: fopen failed");
        return -1;
    }
    
    while (fread(&start_inode, sizeof(uint32_t), 1, file_pointer) == 1 &&
       fread(namebuf, 1, char_buffer, file_pointer) == char_buffer){

        // convert 32 byte name into string 
        int i = 0;
        while (i < char_buffer && namebuf[i] != '\0') {
            i++;
        }   
        
        // allocate memory 
        char *str = malloc(i + 1);
        // malloc checker
        if (str == NULL) {
            perror("malloc failed");
            fclose(file_pointer);
            return -1;
        }

        // copying the bytes from namebuf to str
        memcpy(str, namebuf, i);
        str[i] = '\0';

        if (strcmp(str, target) == 0){
            printf("%s directory found", str);
            free(str);
            fclose(file_pointer);
            return (int)start_inode;
        }

        free(str);

    }
    
    fclose(file_pointer);
    return -1;
                
}

int main(int argc, char*argv[]){

    int current_working_directory = 0; 
    char *directory;  

    int loop_var = 1;
    char input_line[input_buffer];

    const char *possible_inputs[] = {"cd", "mkdir", "touch", "rm"};

    // only want 2 args 
    if (argc == 1){
        printf("Not enough arguments, missing directory");
        return 1;
    }

    if (argc > 2) {
        printf("Too many arguments");
        return 1;
    }

    // set struct vars 
    struct inode_file inode_list[1024];
    int i = 0;

    while (i != 1024){
        inode_list[i].in_use = 0;
        inode_list[i].f_type = 'a';
        inode_list[i].inode_num = -1; 

        i += 1;
    }


    if (argc == 2) {

        // set var equal to user input dir 
        directory = argv[1];


        // check if directory exists
        if (argv[1]){

            // change into directory 
            if(chdir(argv[1]) == 0){

                FILE *input_file = fopen("inodes_list", "rb");
                input_file = fopen("inodes_list", "rb");
                // check if file is valid
                if (input_file == NULL) {
                    perror("Failed to open inodes_list");
                    return 1;
                }


                uint32_t inode_num;
                char f_type;

                int curr = 0;

                while (fread(&inode_num, sizeof(uint32_t), 1, input_file) == 1 && fread(&f_type, sizeof(char), 1, input_file) == 1) {
                    

                    // curr_inode = ch;
                    if ((inode_num > 1023) || (f_type != 'd' && f_type != 'f')) {
                        
                        //stderr error inode not valid 
                        fprintf(stderr, "Inode %d is invalid\n", inode_num);
                    }
                    else {

                        inode_list[inode_num].inode_num = inode_num;
                        inode_list[inode_num].f_type = f_type;
                        inode_list[inode_num].in_use = 1;

                    }

                } 

                //struct inode_file inode_list[0];

                if (inode_list[0].in_use == 1 && inode_list[0].f_type == 'd'){
                    // check if valid 
                    // does nothing if valid
                }

                else{
                    fprintf(stderr, "Inode is invalid\n");
                    exit(1);
                }

                while (1){

                    if (fgets(input_line, input_buffer, stdin) != NULL) {
                        
                        char user_command[input_buffer];
                        char argument[input_buffer];
                        
                        // sscanf will get both the command and the argument (if present)
                        int num_args = sscanf(input_line, "%s %s", user_command, argument);
                        
                        if (strcmp(user_command, "exit") == 0){
                            //exit loop
                            break;
                        }

                        else if (strcmp(user_command, "cd") == 0){
                            if (num_args == 2) {

                                int cd_temp = inode_helper(current_working_directory, inode_list, argument);
                                if (cd_temp != -1){
                                    
                                    if (inode_list[cd_temp].f_type == 'd'){
                                        // update current working directory
                                        current_working_directory = cd_temp;
                                    }
                                    else {
                                        fprintf(stderr, "Error: %s is not a directory\n", argument);
                                    }
                                }
                                else {
                                    fprintf(stderr, "Directory %s not found\n", argument);
                                }
                            }
                            else {
                                fprintf(stderr, "please add an input name\n");
                            }

                        }

                        else if (strcmp(user_command, "ls") == 0){
                            // Create filename from current working directory inode
                            char ls_f_name[16];
                            sprintf(ls_f_name, "%d", current_working_directory);

                            //FILE *ls_file = ls_f_name;
                            FILE *ls_file = fopen(ls_f_name, "rb");

                            if (ls_file == NULL) {
                                perror("Failed to open");
                                //return 1;
                            }
                            else {
                                uint32_t ls_inode;
                                unsigned char ls_name[char_buffer];
                                
                                // Read all entries from the directory
                                while (fread(&ls_inode, sizeof(uint32_t), 1, ls_file) == 1 &&
                                    fread(ls_name, 1, char_buffer, ls_file) == char_buffer) {
                                    
                                        // Convert name to string
                                    int j = 0;
                                    while (j < char_buffer && ls_name[j] != '\0') {
                                        j++;
                                    }
                                    
                                    char *name_str = malloc(j + 1);
                                    if (name_str == NULL) {
                                        perror("malloc failed");
                                        fclose(ls_file);
                                        break;
                                    }
                                    
                                    memcpy(name_str, ls_name, j);
                                    name_str[j] = '\0';
                                    
                                    // Print inode and name
                                    printf("%u %s\n", ls_inode, name_str);
                                    
                                    free(name_str);
                                }
                            }

                            fclose(ls_file);

                        }
                        else if (strcmp(user_command, "mkdir") == 0){
                            if (num_args == 2) {
                                
                                int mkdir_temp = inode_helper(current_working_directory, inode_list, argument);
                                if (mkdir_temp != -1){
                                    fprintf(stderr, "Directory already exists");
                                }
                                else{
                                    
                                    int new_inode = -1;
                                    int temp_int = 0;
                                    while (temp_int < 1024) {
                                        //find not in use inode 
                                        if (inode_list[temp_int].in_use == 0){
                                            new_inode = temp_int;
                                            break;
                                        }

                                        temp_int++;
                                    }

                                    if (new_inode == -1) {
                                        fprintf(stderr, "Error: no available inodes\n");
                                    }
                                    else{
                                        // mark inode as in use
                                        inode_list[new_inode].in_use = 1;
                                        inode_list[new_inode].f_type = 'd';
                                        inode_list[new_inode].inode_num = new_inode;

                                        // Create the file for this inode
                                        char new_dir_filename[16];
                                        sprintf(new_dir_filename, "%d", new_inode);
                                        FILE *new_dir_file = fopen(new_dir_filename, "wb");
                                        
                                        if (new_dir_file == NULL) {
                                            perror("Failed to create directory file");
                                        }
                                        else {
                                            // Write . entry
                                            uint32_t dot_inode = new_inode;
                                            char dot_name[char_buffer];
                                            memset(dot_name, 0, char_buffer);
                                            dot_name[0] = '.';
                                            fwrite(&dot_inode, sizeof(uint32_t), 1, new_dir_file);
                                            fwrite(dot_name, 1, char_buffer, new_dir_file);
                                            
                                            // Write .. entry
                                            uint32_t dotdot_inode = current_working_directory;
                                            char dotdot_name[char_buffer];
                                            memset(dotdot_name, 0, char_buffer);
                                            dotdot_name[0] = '.';
                                            dotdot_name[1] = '.';
                                            fwrite(&dotdot_inode, sizeof(uint32_t), 1, new_dir_file);
                                            fwrite(dotdot_name, 1, char_buffer, new_dir_file);
                                            
                                            fclose(new_dir_file);
                    
                                            // Add entry to current directory
                                            char curr_dir_filename[16];
                                            sprintf(curr_dir_filename, "%d", current_working_directory);
                                            FILE *curr_dir_file = fopen(curr_dir_filename, "ab");

                                            if (curr_dir_file == NULL) {
                                                perror("Failed to update current directory");
                                            }
                                            else {
                                                uint32_t new_entry_inode = new_inode;
                                                char new_entry_name[char_buffer];
                                                memset(new_entry_name, 0, char_buffer);
                                                
                                                // Copy argument name (limit to 32 chars)
                                                int m = 0;
                                                while (m < char_buffer && argument[m] != '\0') {
                                                    new_entry_name[m] = argument[m];
                                                    m++;
                                                }
                                                
                                                fwrite(&new_entry_inode, sizeof(uint32_t), 1, curr_dir_file);
                                                fwrite(new_entry_name, 1, char_buffer, curr_dir_file);
                                                
                                                fclose(curr_dir_file);
                                            }
                                        }
                                    }
                                }
                                
                            }
                        }
                        else if (strcmp(user_command, "touch") == 0){
                            if (num_args == 2) {
                                int touch_temp = inode_helper(current_working_directory, inode_list, argument);
                                if (touch_temp != -1){
                                    // do nothing
                                }

                                else{
                                    
                                    int new_inode = -1;
                                    int temp_int_2 = 0;
                                    while (temp_int_2 < 1024) {
                                        //find not in use inode 
                                        if (inode_list[temp_int_2].in_use == 0){
                                            new_inode = temp_int_2;
                                            break;
                                        }

                                        temp_int_2++;
                                    }

                                    if (new_inode == -1) {
                                        fprintf(stderr, "Error: no available inodes\n");
                                    }
                                    else{
                                        // mark inode as in use
                                        inode_list[new_inode].in_use = 1;
                                        inode_list[new_inode].f_type = 'f';
                                        inode_list[new_inode].inode_num = new_inode;
                                    
                                    }
                                    
                                }
                            }
                        }
                    }
                }
    

                printf("Changed into directory %s\n", argv[1]);
            }

            else{
                printf("Failed to change into directory");
            }
        }
    }


}
