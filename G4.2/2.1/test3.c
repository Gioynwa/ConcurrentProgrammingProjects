#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "mybsem.h"
#define MAX_SIZE 10

volatile mybsem sem;

void *row();

int main(int argc, char *argv[]) {
	
	
    pthread_t thr[MAX_SIZE];
    int i, check ;

    mybsem_init((mybsem *)&sem,1);
    
    for (i = 0; i < MAX_SIZE;i ++){
            
        pthread_create(&thr[i],NULL,(void*)row,NULL);
            
    }
    
    sleep(70);
    check = mybsem_destroy((mybsem *)&sem);
    if(check != 0){
        perror("");
    }
    
    return(0);
}


void *row(){
	
    int i;
    
    printf("the id : %lu \n",pthread_self());
    mybsem_down((mybsem*)&sem);
    
    for(i = 0; i<5; i++) {
            printf("the id : %lu \n",pthread_self());
            sleep(1);
    }
    
    printf("\n");
    mybsem_up((mybsem*)&sem);
	
	
}