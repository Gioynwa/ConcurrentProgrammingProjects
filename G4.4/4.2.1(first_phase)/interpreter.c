#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "hw4.h"

#define MAX 6

volatile int counter_global = 0, l = 0, noftests;

void *run_thread() {
    
    
    char *ptr1;
    char ptr2[10] = {"\0"};
    int f,conv_to_string,check_args_help = 0,check_global_help = 0,check_local_help = 0;
    int number_of_args = 0, size = 0;
    char **print_args;
    struct list *temp;
    int k = 0;
    int num1, num2, value, check = 1;
    int i, j, check_local, check_global, check_label, check_args, orig_check_local;
    char v[2], magic_number[7] = "#SIMBA\0", c, space[2] = " ", str1[10] = {"\0"}, str2[10] = {"\0"};
    int isArg = 0, num = 0, s = 0, r = 0, p = 0, q = 0, isprint;
    char oper[3] = {"\0"} , string[100] = {"\0"}, str[100] = {"\0"};
    char *instruction, *help, *ptr;
    
    /*=============================*/
    struct multithreading *buf;
    /*=============================*/
    
    buf = (struct multithreading*)malloc(sizeof(struct multithreading));
    if(buf == NULL) {
        write(1, "allocation error\n", 17);
        exit(1);
    }
    buf[num].flag = 0;
    
    
    do{
        if(buf[num].flag == 0){
            buf[num].first_time = 1;
            buf[num].flag = 1;
            buf[num].going_to_label = 0;
            buf[num].counter_args = 0;
            buf[num].counter_labels = 0;
            buf[num].counter_local = 0;
            
            buf[num].local_var = (struct variables*)malloc(sizeof(struct variables));
            if(buf[num].local_var == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            
            buf[num].label_array = (struct labels*)malloc(sizeof(struct labels));
            if(buf[num].label_array == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            
            buf[num].arg_array = (struct args*)malloc(5*sizeof(struct args));
            if(buf[num].arg_array == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            
            /* get the first token */
            /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
            strcpy(string, strtok((char*)threads_info[num].name, space));
            
            help = strtok(NULL, space);
            
            /* walk through other tokens */
            while(help != NULL) {
                
                if(buf[num].counter_args > 1){
                    help = strtok(NULL, space);
                    if(help == NULL) {
                        break;
                    }
                }
                
                if((help != NULL) && (buf[num].counter_args != 0)) {
                    strcpy(string, help);
                }
                
                strcpy(buf[num].arg_array[buf[num].counter_args].name,"$argv");
                if(buf[num].counter_args == 0){
                    strcpy(buf[num].arg_array[buf[num].counter_args].content, string);
                }
                else{
                    
                    buf[num].arg_array[buf[num].counter_args].val = atoi(string);
                }
                buf[num].arg_array[buf[num].counter_args].pointer = buf[num].counter_args;
                
                buf[num].counter_args++;
                
            }
            
            if(buf[num].counter_args == 0) {
                
                strcpy(buf[num].arg_array[buf[num].counter_args].content,strtok((char*)threads_info[num].name,"\n"));
                strcpy(buf[num].arg_array[buf[num].counter_args].name,"$argv");
                buf[num].arg_array[buf[num].counter_args].pointer = buf[num].counter_args;
                
                buf[num].counter_args++;
                
            }
            
            
            strcpy(buf[num].local_var[buf[num].counter_local].name, "$argc");
            buf[num].local_var[buf[num].counter_local].val = buf[num].counter_args;
            
            buf[num].counter_local++;
            buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
            if(buf[num].local_var == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            
            
            /*=====================================*/
            //take all the contents of the given program and put them into an array
            threads_info[num].val = open(buf[num].arg_array[0].content, O_RDONLY);
            if(threads_info[num].val == -1) {
                    write(1, "prog does not exist\n", 20);
                    exit(1);
            }
        }
    
        j = 0;
        
        do{
            check = read(threads_info[num].val, &c, sizeof(char));
            
            if((check == 0)||(c == '\n')){
                break;
            }
            
            if(j == 0) {
                instruction = (char*)malloc(sizeof(char));
                instruction[j] = c;
                j++;
            }
            else {
                
                instruction = (char*)realloc(instruction, (j+1)*sizeof(char));
                instruction[j] = c;
                j++;
            }
            
           
        }while(c != '\n');
        
        if(check == 0){
            break;
        }
        
        /*replace \n with \0*/
        instruction[j-1] = '\0';
        /*this size needed for branches*/
        size = strlen(instruction) + 2*sizeof(char);
        
        if(buf[num].first_time == 1) {
            buf[num].first_time = 0;
            if(strcmp(instruction, magic_number) != 0) {
                write(1, "The program is not supported\n", sizeof(char)*29);
                exit(1);
            }
        }
        else {
            
            /* get the first token */
            
            strcpy(str, strtok(instruction, space));
            write(1, str, strlen(str));
            write(1, " ", sizeof(char));
            value = search(str);
            
            if(value == -1) {
                write(STDOUT_FILENO, "Syntax error!\n", 15);
                exit(1);
            }
            else if(value == 20) {
                
                strcpy(buf[num].label_array[buf[num].counter_labels].name, str);
                buf[num].label_array[buf[num].counter_labels].pos = lseek(threads_info[num].val, 0, SEEK_CUR) - size;
                buf[num].counter_labels++;
                
                buf[num].label_array = (struct labels*)realloc(buf[num].label_array,(buf[num].counter_labels+1)*sizeof(struct labels));
                if(buf[num].label_array == NULL) {
                        write(1, "allocation error\n", 17);
                        exit(1);
                }
                
                if(strcmp(str, buf[num].str_label) == 0) {
                        buf[num].going_to_label = 0;
                }

                strcpy(str, strtok(NULL, space));
				
                value = search(str);
                write(1,str,strlen(str));
                write(1,"\n",sizeof(char));
            }
            
            if(buf[num].going_to_label == 0) {
                    switch(value) {
                        case 1:
                                strcpy(str, strtok(NULL, space));
                                if(str[0] != '$') {
                                        write(STDOUT_FILENO, "Syntax error in load!\n", 22);
                                        exit(1);
                                }
                                write(1, str, strlen(str));
                                write(1, " ", sizeof(char));
                                check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                
                                if(check_local == -1) {
                                        check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                        if(check_args == -1){
                                            if(strncmp(str,"$argv",5) == 0){
                                                strcpy(buf[num].arg_array[buf[num].counter_args].name,"$argv");
                                                v[0] = str[6];
                                                v[1] = '\0';
                                                buf[num].arg_array[buf[num].counter_args].pointer = atoi(v);
                                                buf[num].counter_args++;
                                                isArg = 1;
                                            }
                                            else{
                                                strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                                buf[num].counter_local++;
                                            }
                                        }
                                }
                                
                                strcpy(str, strtok(NULL, "\n"));
                                write(1, str, strlen(str));
                                write(1,"\n",sizeof(char));
                                if(str[0] != '$') {
                                        write(STDOUT_FILENO, "Syntax error in load!\n", 22);
                                        exit(1);
                                }
                                
                                /*in this section stores variable like $b[$a]*/
                                /*============================================*/
                                ptr1 = strchr(str,'[');
                                
                                if(ptr1 != NULL) {
                                
                                
                                    ptr1 = ptr1 + 1;
                                    
                                    if(*ptr1 == '$'){
                                        
                                        f = 0;
                                        while(*ptr1 != ']'){
                                            
                                            ptr2[f] = *ptr1;
                                            ptr1 = ptr1 + 1;
                                            f++;
                                            
                                        }
                                        ptr2[f] = '\0';
                                        
                                        
                                        
                                        check_args_help = search_args(ptr2,buf[num].arg_array,buf[num].counter_args);
                                        check_local_help = search_varname(ptr2, buf[num].local_var, buf[num].counter_local);
                                        check_global_help = search_varname(ptr2, (struct variables *)global_var, counter_global);
                                        
                                        if(check_args_help != -1){
                                            conv_to_string = buf[num].arg_array[check_args_help].val;
                                        }
                                        else if (check_local_help != -1){
                                            conv_to_string = buf[num].local_var[check_local_help].val;
                                        }
                                        else if ( check_global_help != -1){
                                            conv_to_string = global_var[check_global_help].val;
                                        }
                                        else{
                                            write(1,"undefined\n",10);
                                        }
                                        
                                        
                                        i = 0;
                                        while(str[i] != '['){
                                            i++;
                                        }
                                        
                                        
                                        
                                        sprintf(ptr2 , "%d", conv_to_string);
                                        strcpy(ptr2,ptr2);
                                        
                                        i++;
                                        str[i] = ptr2[0];
                                        i++;
                                        
                                        j = 1;
                                        while (j < strlen(ptr2)) {
                                            str[i] = ptr2[j];
                                            j++;
                                            i++;
                                        }
                                        
                                        
                                        str[i] = ']';
                                        str[i+1] = '\0';
                                        
                                    }
                                }
                                /*============================================*/
                                
                                check_global = search_varname(str, (struct variables *)global_var, counter_global);
                                
                                if(check_global == -1) {
                                        write(1, "global_var does not exist\n", 26);
                                        exit(1);
                                }
                                else {
                                    if(check_local == -1){
                                        if(check_args == -1){
                                            if(isArg != 1){
                                                buf[num].local_var[buf[num].counter_local-1].val = global_var[check_global].val;
                                                buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                                if(buf[num].local_var == NULL) {
                                                        write(1, "allocation error\n", 17);
                                                        exit(1);
                                                }
                                            }
                                            else{
                                                isArg = 0;
                                                buf[num].arg_array[buf[num].counter_args-1].val = global_var[check_global].val;
                                                buf[num].arg_array = (struct args*)realloc(buf[num].arg_array,(buf[num].counter_args+1)*sizeof(struct args));
                                                if(buf[num].arg_array == NULL) {
                                                        write(1, "allocation error\n", 17);
                                                        exit(1);
                                                }
                                            }
                                        }
                                        else{
                                            buf[num].arg_array[check_args].val = global_var[check_global].val;
                                        }
                                    }
                                    else{
                                        
                                        buf[num].local_var[check_local].val = global_var[check_global].val;
                                        
                                        
                                    }
                                    write(1,"load execution ok.\n",19);
                                }
                                
                                break;
                                    
                            case 2:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in store!\n", 23);
                                            exit(1);
                                    }
                                    
                                    /*in this section stores variable like $b[$a]*/
                                    /*============================================*/
                                    ptr1 = strchr(str,'[');
                                    
                                    if(ptr1 != NULL) {
                                    
                                    
                                        ptr1 = ptr1 + 1;
                                        
                                        if(*ptr1 == '$'){
                                            
                                            f = 0;
                                            while(*ptr1 != ']'){
                                                
                                                ptr2[f] = *ptr1;
                                                ptr1 = ptr1 + 1;
                                                f++;
                                                
                                            }
                                            ptr2[f] = '\0';
                                            
                                            
                                            
                                            check_args_help = search_args(ptr2,buf[num].arg_array,buf[num].counter_args);
                                            check_local_help = search_varname(ptr2, buf[num].local_var, buf[num].counter_local);
                                            check_global_help = search_varname(ptr2, (struct variables *)global_var, counter_global);
                                            
                                            if(check_args_help != -1){
                                                conv_to_string = buf[num].arg_array[check_args_help].val;
                                            }
                                            else if (check_local_help != -1){
                                                conv_to_string = buf[num].local_var[check_local_help].val;
                                            }
                                            else if ( check_global_help != -1){
                                                conv_to_string = global_var[check_global_help].val;
                                            }
                                            else{
                                                write(1,"undefined\n",10);
                                            }
                                            
                                            
                                            i = 0;
                                            while(str[i] != '['){
                                                i++;
                                            }
                                            
                                            sprintf(ptr2 , "%d", conv_to_string);
                                            strcpy(ptr2,ptr2);
                                            
                                            i++;
                                            str[i] = ptr2[0];
                                            i++;
                                            
                                            j = 1;
                                            while (j < strlen(ptr2)) {
                                                str[i] = ptr2[j];
                                                j++;
                                                i++;
                                            }
                                            
                                            
                                            str[i] = ']';
                                            str[i+1] = '\0';
                                            
                                        }
                                    }
                                    /*============================================*/
                                    
                                    check_global = search_varname(str, (struct variables *)global_var, counter_global);
                                    
                                    if(check_global == -1) {
                                            
                                            strcpy((char*)global_var[counter_global].name,str);
                                            counter_global++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    if(check_global == -1){
                                            global_var[counter_global-1].val = num1;
                                    }
                                    else{
                                            global_var[check_global].val = num1;
                                    }
                                    
                                    write(1,"store execution ok.\n",20);
                                    
                                    global_var = (struct variables*)realloc((struct variables *)global_var,(counter_global+1)*sizeof(struct variables));
                                    if(global_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                            
                            case 3:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in set!\n", 21);
                                            exit(1);
                                    }
                                    
                                    orig_check_local = search_varname(str, buf[num].local_var,buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1;
                                    }
                                    
                                    write(1, "set execution ok.\n",18);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                                    
                            case 4:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in add!\n", 21);
                                            exit(1);
                                    }
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    orig_check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1 + num2;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1 + num2;
                                    }
                                    
                                    write(1,"add execution ok.\n",17);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                                    
                            case 5:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in sub!\n", 21);
                                            exit(1);
                                    }
                                    orig_check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1 - num2;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1 - num2;
                                    }
                                    
                                    write(1,"sub execution ok.\n",17);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                            
                            case 6:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in mul!\n", 21);
                                            exit(1);
                                    }
                                    orig_check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1*num2;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1*num2;
                                    }
                                    
                                    write(1,"mul execution ok.\n",17);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                            
                            case 7:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in div!\n", 21);
                                            exit(1);
                                    }
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    orig_check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1/num2;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1/num2;
                                    }
                                    
                                    write(1,"div execution ok.\n",17);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                            
                            case 8:
                                    
                                    strcpy(str, strtok(NULL, space));
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in mod!\n", 21);
                                            exit(1);
                                    }
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    orig_check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                    
                                    if(orig_check_local == -1) {
                                            strcpy(buf[num].local_var[buf[num].counter_local].name, str);
                                            buf[num].counter_local++;
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    if(orig_check_local == -1) {
                                            buf[num].local_var[buf[num].counter_local-1].val = num1%num2;
                                    }
                                    else{
                                            buf[num].local_var[orig_check_local].val = num1%num2;
                                    }
                                    
                                    write(1,"mod execution ok.\n",18);
                                    
                                    buf[num].local_var = (struct variables*)realloc(buf[num].local_var,(buf[num].counter_local+1)*sizeof(struct variables));
                                    if(buf[num].local_var == NULL) {
                                            write(1, "allocation error\n", 17);
                                            exit(1);
                                    }
                                    
                                    break;
                            
                            case 9:
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(num1 > num2) {
                                            check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                            if(check_label == -1) {
                                                    buf[num].going_to_label = 1;
                                                    strcpy(buf[num].str_label, str);
                                            }
                                            else {
                                                    lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                            }
                                    }
                                    
                                    write(1,"brgt execution ok.\n",19);
                                    break;
                                    
                            case 10:
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(num1 >= num2) {
                                            check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                            if(check_label == -1) {
                                                    buf[num].going_to_label = 1;
                                                    strcpy(buf[num].str_label, str);
                                            }
                                            else {
                                                    lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                            }
                                    }
                                    
                                    write(1,"brge execution ok.\n",19);
                                    
                                    break;
                            
                            case 11:
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(num1 < num2) {
                                            check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                            if(check_label == -1) {
                                                    buf[num].going_to_label = 1;
                                                    strcpy(buf[num].str_label, str);
                                            }
                                            else {
                                                    lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                            }
                                    }
                                    
                                    break;
									
                                    write(1,"brlt execution ok.\n",18);
                            
                            case 12:
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(num1 <= num2) {
                                            check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                            if(check_label == -1) {
                                                    buf[num].going_to_label = 1;
                                                    strcpy(buf[num].str_label, str);
                                            }
                                            else {
                                                    lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                            }
                                    }
                                    
                                    write(1,"brle execution ok.\n",18);
									
                                    break;
                                    
                            
                            case 13:
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                        num1 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num1 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, space));
                                    
                                    write(1,str,strlen(str));
                                    write(1," ",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                        check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                        if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                        write(1, str, strlen(str));
                                                        write(1, " undefined.\n", 12);
                                                        exit(1);
                                                }
                                                else {
                                                    num2 = buf[num].arg_array[check_args].val;
                                                }
                                        }
                                        else { 
                                                num2 = buf[num].local_var[check_local].val;
                                        }
                                    }
                                    else {
                                            num2 = atoi(str);
                                    }
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(num1 == num2) {
                                            check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                            if(check_label == -1) {
                                                    buf[num].going_to_label = 1;
                                                    strcpy(buf[num].str_label, str);
                                            }
                                            else {
                                                    lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                            }
                                    }
                                    
                                    write(1,"breq execution ok.\n",19);
                                    
                                    break;
                            
                            case 14:
                                    
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    check_label = search_labels(str, buf[num].label_array, buf[num].counter_labels);
                                    if(check_label == -1) {
                                            buf[num].going_to_label = 1;
                                            strcpy(buf[num].str_label, str);
                                    }
                                    else {
                                            
                                            lseek(threads_info[num].val, buf[num].label_array[check_label].pos, SEEK_SET);
                                    }
                                    
                                    write(1,"bra execution ok.\n",18);
                                    
                                    break;
                            
                            case 15:
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in store!\n", 23);
                                            exit(1);
                                    }
                                    
                                    check_global = search_varname(str, (struct variables *)global_var, counter_global);
                                    
                                    if(check_global == -1) {
                                            strcpy((char*)global_var[counter_global].name,str);
                                            strcpy(node->name_of_sem,str);
                                            counter_global++;
                                            global_var[counter_global-1].val = 1;
                                    }
                                    else{
                                        
                                        strcpy(node->name_of_sem,str);
                                        if(global_var[check_global].val <= 0){
                                        
                                            node->flag_sem = 0;
                                            write(1,"down execution ok.\n",18);
                                            break;
                                        }
                                        
                                    }
                                    
                                    if(check_global != -1){
                                        global_var[check_global].val--; 
                                    }
                                    write(1,"down execution ok.\n",18);
                                    
                                    break;
                            case 16:
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] != '$') {
                                            write(STDOUT_FILENO, "Syntax error in up!\n", 20);
                                            exit(1);
                                    }
                                    
                                    check_global = search_varname(str, (struct variables *)global_var, counter_global);
                                    
                                    if(check_global == -1) {
                                            strcpy((char*)global_var[counter_global].name,str);
                                            strcpy(node->name_of_sem,str);
                                            counter_global++;
                                            global_var[counter_global-1].val = 0;
                                    }
                                    else{
                                        
                                        global_var[check_global].val++;
                                        temp = node->next;
                                        while((temp->flag_sem == 0) || (temp != node)) {
                                            if(strcmp(temp->name_of_sem, (char*)global_var[check_global].name) == 0) {
                                                if(global_var[check_global].val > 0) {
                                                    temp->flag_sem = 1;
                                                    write(1,"up execution ok.\n",16);
                                                    break;
                                                }
                                                
                                            }
                                            temp = temp->next;
                                        }
                                        
                                    }
                                    write(1,"up execution ok.\n",16);
                                    break;
                            case 17:
                                    strcpy(str, strtok(NULL, "\n"));
                                    
                                    write(1,str,strlen(str));
                                    write(1,"\n",sizeof(char));
                                    
                                    if(str[0] == '$') {
                                            check_local = search_varname(str, buf[num].local_var, buf[num].counter_local);
                                            if(check_local == -1) {
                                                check_args = search_args(str,buf[num].arg_array,buf[num].counter_args);
                                                if(check_args == -1) {
                                                    write(1, str, strlen(str));
                                                    write(1, " undefined.\n", 12);
                                                    exit(1);
                                                }
                                                else {
                                                    num1 = buf[num].arg_array[check_args].val;
                                                }
                                            }
                                            else { 
                                                    num1 = buf[num].local_var[check_local].val;
                                            }
                                    }
                                    else {
                                            num1 = atoi(str);
                                    }
                                    
                                    write(1,"sleep execution ok.\n",19);
                                    clock_t endwait;
	
                                    endwait = clock () + num1 * CLOCKS_PER_SEC;
                                    while (clock() < endwait) {}
                                    
                                    break;
                            case 18:
                                
                                write(1, "\n", sizeof(char));
                                oper[0] = '"';
                                
                                strcpy(str, strtok(NULL, oper));
                                write(1, str, strlen(str));
                                
                                help = strtok(NULL,"\n");
                                ptr = help;
                                
                                number_of_args = 0;
                                
                                while(help != NULL){
                                    
                                    help = strchr(help,' ');
                                    
                                    if(help != NULL){
                                        help = help + 1;
                                        
                                    }
                                    number_of_args++;
                                }
                                
                                print_args = (char**)malloc(number_of_args*sizeof(print_args));
                                
                                
                                isprint = 0;
                                p = 0;
                                
                                while(p < number_of_args - 1 ) {
                                    
                                    
                                    help = strchr(ptr, ' ');
                                    
                                    
                                    if(help != NULL){
                                        
                                        help = help + 1;
                                        ptr = help;
                                        
                                        strcpy(str,help);
                                        str[strlen(str)] = ' ';
                                        str[strlen(str)+1] = '\0';
                                        
                                        isprint = 1;
                                       
                                        i = 0;
                                        while(str[i] != ' '){
                                            
                                            str1[i] = str[i];
                                            i++;
                                        }
                                        
                                        print_args[p] = (char*)malloc(sizeof(char)*(i+1));
                                        strcpy(print_args[p],str1);
                                        for(i = 0; i < 20; i++) {
                                            str1[i] = '\0';
                                        }
                                        
                                    }
                                    
                                    p++;
                                }
                                
                                
                                
                                p = 0;
                                while(p < number_of_args - 1) {
                                    
                                        if(print_args[p][0] == '$') {
                                                            
                                            check_local = search_varname(print_args[p], buf[num].local_var, buf[num].counter_local);
                                            if(check_local == -1) {
                                                    check_args = search_args(print_args[p],buf[num].arg_array,buf[num].counter_args);
                                                    if(check_args == -1) {
                                                            write(1, print_args[p], strlen(print_args[p]));
                                                            write(1, " undefined.\n", 12);
                                                            exit(1);
                                                    }
                                                    else {
                                                            num2 = buf[num].arg_array[check_args].val;
                                                    }
                                            }
                                            else { 
                                                    num2 = buf[num].local_var[check_local].val;
                                            }
                                        }
                                        else {
                                            num2 = atoi(print_args[p]);
                                        }
                                            
                                        dprintf(1, " %d", num2);
                                        p++;
                                }
                                
                                
                                write(1,"\nprint execution ok.\n",20);
                                break;
                            case 19:
                                    noftests--;
                                    if(noftests == 0){
                                        
                                        for(i = 0; i < l; i++){
                                            threads_info[i].val = 0;
                                            for(j = 0; j < 100; j++){
                                                threads_info[i].name[j] = '\0';
                                            }
                                        }
                                        
                                        l = 0;
                                        
                                        free(instruction);
                                        free(buf);
                                        
                                    }
                                    
                                    remove_node(node);
                                    write(1,"return execution ok.\n",21);
                                    return(0);
                                    break;
                            default:
                                    break;
                    }
            }
        }
        
        
        for(i = 0; i < buf[num].counter_local; i++) {
            write(1,"locals ",7*sizeof(char));
            write(1,buf[num].local_var[i].name,strlen(buf[num].local_var[i].name));
            write(1,":",sizeof(char)); 
            dprintf(1, "%d\n", buf[num].local_var[i].val);
        }
        
        for(i = 0; i < counter_global; i++) {
            write(1,"globals ",8*sizeof(char));
            write(1,(char*)global_var[i].name,strlen((char*)global_var[i].name));
            
            write(1,":",sizeof(char)); 
            dprintf(1, "%d", global_var[i].val);
            
            write(1,"\n",sizeof(char));
        }	
        for(i = 0; i < buf[num].counter_args; i++) {
            if(i == 0){
                write(1, "argvs ", 6*sizeof(char));
                write(1, buf[num].arg_array[i].name, strlen(buf[num].arg_array[i].name));
                write(1, ":", sizeof(char));
                write(1, buf[num].arg_array[i].content, strlen(buf[num].arg_array[i].content));
                write(1, "\n", sizeof(char));
            }
            else{
                write(1, "argvs ", 6*sizeof(char));
                write(1, buf[num].arg_array[i].name, strlen(buf[num].arg_array[i].name));
                write(1, ":", sizeof(char));
                dprintf(1, "%d", buf[num].arg_array[i].val);
                write(1, "\n", sizeof(char));
            }
         }
         
        free(instruction);
        sleep(2);
        
        temp = node->next;
        while(temp->flag_sem == 0){
            
            
            temp = temp->next;
            
        }
        
        if((node != node->next)) {
            
            node = temp;
            num = node->nofpos;
            
            buf = (struct multithreading*)realloc(buf, (l+1)*sizeof(struct multithreading));
            if(buf == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            if(buf[num].flag == 1) {
            }
            else {
                buf[num].flag = 0;
            }
            
        }
        
    }while(1);
    
}


int main(int argc, char *argv[]) {
    
    struct list* curr;
    struct list* head;
    pthread_t thread_id;
    int k = 0, nofthreads = 1;
    char c;
    char *test_command;
    
    if(argc != 2) {
        write(1, "Wrong number of arguments\n", 26);
        exit(1);
    }
    
    threads_info = (struct variables*)malloc(sizeof(struct variables));
    if(threads_info == NULL) {
        write(1, "allocation error\n", 17);
        exit(1);
    }
    
    global_var = (struct variables*)malloc(sizeof(struct variables));
    if(global_var == NULL) {
        write(1, "allocation error\n", 17);
        exit(1);
    }
    
    noftests = 0;
    
  
    do {
        
        read(STDIN_FILENO, &c, sizeof(char));
        
        if(k == 0) {
            test_command = (char*)malloc(sizeof(char));
            test_command[k] = c;
            k++;
        }
        else {
            test_command = (char*)realloc(test_command, (k+1)*sizeof(char));
            test_command[k] = c;
            k++;
            
        }
        
        if(c == '\n') {
            
            noftests++;
            
            if(l != 0) {
                add(head, curr, l);
                
            }
            else{
                nofthreads = 1;
                head = init(head);
                node = head;
            }
            test_command[k - 1] = '\0';
            strcpy((char*)threads_info[l++].name, test_command);
            if(nofthreads <= atoi(argv[1])) {
                pthread_create(&thread_id, NULL, (void*)run_thread, NULL);
                nofthreads++;
            }
            k = 0;
            threads_info = (struct variables*)realloc((struct variables*)threads_info, (l+1)*sizeof(struct variables));
            if(threads_info == NULL) {
                write(1, "allocation error\n", 17);
                exit(1);
            }
            free(test_command);
            
        }
        
    }while(1);
    
    
    pthread_join(thread_id,NULL);
    
    /*=====================================*/
    
    return(0);
}

