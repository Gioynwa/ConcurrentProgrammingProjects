#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "hw4.h"

int search(char *command) {
	
    
    if(strcmp(command, "LOAD") == 0) {
        return(1);
    }
    else if(strcmp(command, "STORE") == 0) {
        return(2);
    }
    else if(strcmp(command, "SET") == 0) {
        return(3);
    }
    else if(strcmp(command, "ADD") == 0) {
        return(4);
    }
    else if(strcmp(command, "SUB") == 0) {
        return(5);
    }
    else if(strcmp(command, "MUL") == 0) {
        return(6);
    }
    else if(strcmp(command, "DIV") == 0) {
        return(7);
    }
    else if(strcmp(command, "MOD") == 0) {
        return(8);
    }
    else if(strcmp(command, "BRGT") == 0) {
        return(9);
    }
    else if(strcmp(command, "BRGE") == 0) {
        return(10);
    }
    else if(strcmp(command, "BRLT") == 0) {
        return(11);
    }
    else if(strcmp(command, "BRLE") == 0) {
        return(12);
    }
    else if(strcmp(command, "BREQ") == 0) {
        return(13);
    }
    else if(strcmp(command, "BRA") == 0) {
        return(14);
    }
    else if(strcmp(command, "DOWN") == 0) {
        return(15);
    }
    else if(strcmp(command, "UP") == 0) {
        return(16);
    }
    else if(strcmp(command, "SLEEP") == 0) {
        return(17);
    }
    else if(strcmp(command, "PRINT") == 0) {
        return(18);
    }
    else if(strcmp(command, "RETURN") == 0) {
        return(19);
    }
    else if(strncmp(command, "L", 1) == 0){
        return(20);
    }
    else {
        return(-1);
    }
     
}
    
int search_varname(char* str, struct variables* array, int size) {
    
    int i;
    
    for(i = 0; i < size; i++) {
        if(strcmp(array[i].name, str) == 0) {
            return(i);
        }
    }
    
    return(-1);
    
}

int search_labels(char* str, struct labels* array, int size) {
    
    int i;
    
    for(i = 0; i < size; i++) {
        if(strcmp(array[i].name, str) == 0) {
            return(i);
        }
    }
    
    return(-1);
    
}

int search_args(char* str, struct args* array, int size) {
    
    int i;
    char n[2];
    
    n[0] = str[6];
    n[1] = '\0';
    
    for(i = 0; i < size; i++) {
        if((strncmp(array[i].name, str,5) == 0) && (array[i].pointer == atoi(n))){
            return(i);
        }
    }
    
    return(-1);
    
}

struct list* init(struct list* head){
    
    head = (struct list*)malloc(sizeof(struct list));
    if(head == NULL){
        return(head);
    }
    else{
        head->nofpos = 0;
        head->flag_sem = 1;
        strcpy(head->name_of_sem,"\0");
        head->next = head;
        head->prev = head;
    }
    
    return(head);
}


void remove_node(struct list* curr){
    
    struct list *current;
    
    if ((curr->next == curr) && (curr->prev == curr)){
        /*here we have to do l = 0*/
        free(curr);
        
    }
    else{
    
        for(current = curr; current->next != curr; current = current->next) {}
        if(current == curr) {
            free(curr);
        }
        else {
            current->next = curr->next;
            curr->next->prev = current;
            free(curr);
        }
    }
    
}



void add(struct list* head, struct list* curr , int position){
    
    struct list *current;
    
    curr = (struct list*)malloc(sizeof(struct list));
    if(curr == NULL){
        exit(1);
    }
    
    if((head->next == head) && (head->prev == head)){
        
        
        head->next = curr;
        head->prev = curr;
        curr->next = head;
        curr->prev = head;
        curr->nofpos = position;
        curr->flag_sem = 1;
        strcpy(curr->name_of_sem,"\0");
    }
    else{
    
        for(current = head; current->next != head; current = current->next) {}
        
        current->next = curr;
        curr->prev = current;
        curr->next = head;
        head->prev = curr;
        curr->nofpos = position;
        curr->flag_sem = 1;
        strcpy(curr->name_of_sem,"\0");
    
    }
   
}
