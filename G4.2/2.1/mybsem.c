#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mybsem.h"


int mybsem_init(mybsem *sem, int number) {
	
    int check, mtxtype;
    pthread_mutexattr_t attr;
	
    (*sem).value = number;
    
    check = pthread_mutex_init(&((*sem).locker), NULL);
    
    return(check);	
		
}


int mybsem_destroy(mybsem *sem) {
	
    int check;
	
    check = pthread_mutex_destroy(&((*sem).locker));
    return(check);
	
}


int mybsem_down(mybsem *sem) {
	
    int check;
    
    check = pthread_mutex_lock(&((*sem).locker));
    
    if((*sem).value == 0) {
        check = pthread_mutex_lock(&((*sem).locker));
    }
    

    (*sem).value = 0;
	

    return(check);
	
}


int mybsem_up(mybsem *sem) {
	
    int check = 0;
	
    if((*sem).value == 0) {
        (*sem).value = 1;
        check = pthread_mutex_unlock(&((*sem).locker));
        if(check != 0) {
                printf("ERROR UNLOCK\n");
        }
    }
            
    return(check);
	
}
	
		
		
		
		
