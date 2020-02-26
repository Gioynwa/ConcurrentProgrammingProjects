#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "mybsem.h"


volatile mybsem sem;

void *row();
void *column();

int main(int argc, char *argv[]) {
	
    pthread_t thr1, thr2;
    int i,check ;
	
    mybsem_init((mybsem *)&sem,0);
	
    pthread_create(&thr1,NULL,(void*)row,NULL);
    pthread_create(&thr2,NULL,(void*)column,NULL);
    
    
    sleep(10);
    mybsem_up((mybsem*)&sem);
    check = mybsem_destroy((mybsem *)&sem);
    if(check != 0){
        perror("");
    }
	
    return(0);
}


void *row(){
	
	
    mybsem_down((mybsem*)&sem);
	
    printf("the id : %lu row\n",pthread_self());
	
	  
	
}


void *column(){
	
    int i;
    
    for(i = 1; i < 5;i++){
        printf("the id : %lu column\n",pthread_self());
        sleep(1);
    }
	
	
    mybsem_up((mybsem*)&sem);
    
	
}
