#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ccr.h"

volatile int max_passengers, nofpass = 0, flag = 0, first_time = 1, counter = 0;
volatile int val_start = 0, val_rest = 0, val_train_wait = 0, flag_in = 1, val_locker = 0;

CCR_DECLARE(rest)
CCR_DECLARE(start)
CCR_DECLARE(train_wait)
CCR_DECLARE(locker)

void *train_ride(char *c);

int main(int argc, char *argv[]) {
    
    char select;
    int check;
    pthread_t thread;
    
    if(argc != 2) {
        printf("The number of arguments must be 2\n");
        exit(1);
    }
    
    max_passengers = atoi(argv[1]);
    
    /*=========================INITIALIZATIONS=================================*/
    CCR_INIT(rest)
    CCR_INIT(start)
    CCR_INIT(train_wait)
    CCR_INIT(locker)
    /*============================================================================*/
	
    printf("First press 't' for train, then 'p' for passengers or 'q' for exit.\n");
    
    while(1) {

        scanf(" %c", &select);
        
        if((select == 'p') || (select == 't')) {
            check = pthread_create(&thread, NULL, (void*)train_ride, (void*)&select);
            if(check != 0) {
                perror("");
                exit(1);
            }
        }
        if(select == 'q') {
            flag = 1;
            break;
        }
    }
    
    pthread_join(thread, NULL);
    
    return(0);
    
}


void *train_ride(char *c) {
    
    char select;
    int i;
    
    select = *c;
    
    if(select == 'p'){
        counter++;
        if(counter > max_passengers) {
            //the rest passengers have to wait their time
            if(flag_in == 1) {
                    /*===================================================*/
                    CCR_EXEC(locker,(val_locker == 1), val_locker = 1)
                    /*===================================================*/
            }
 			
            /*=======================================================*/
            CCR_EXEC(rest, (val_rest == 1), val_rest = 1)
            /*=======================================================*/
        }
    }
    
    while(1){
        if((select == 'p')) {
            nofpass++;
            /*when the last passenger come each time the train starts*/
            if(nofpass%max_passengers == 0) {
                /*=================================================*/
                CCR_EXEC(start, (1), val_start = 1)
                /*=================================================*/
            }
        }
            
            
        if(select == 't'){
            printf("train is waiting for passengers to come,train id is : %lu\n",pthread_self());
        }
        
        if((first_time == 0) && (select == 't')) {
            /*if other passengers wait the train takes them the right time*/
                val_locker = 0;
                val_rest = 0;
                val_start = 0;
                val_train_wait = 0;
                
                if(flag_in == 1) {
                    flag_in = 0;
                    /*====================================================*/ 
                    CCR_EXEC(locker, (1), val_locker = 1)
                    /*====================================================*/ 
                }

            /*==================================================*/
            CCR_EXEC(rest, (1), val_rest = 1)
            /*==================================================*/
        }
        
        /*train and passengers must be together*/
        /*=====================================================*/
        CCR_EXEC(start, (val_start == 1), val_start = 1)
        /*=====================================================*/
		if((select == 't') && (first_time == 0) && (flag_in == 0)) {
			
                    /*======================================================*/ 
                    CCR_EXEC(locker,(val_locker == 1), val_locker = 1)
                    /*======================================================*/ 
                    val_locker = 0;
                    flag_in = 1;
		}
        
        /*if selection is q ( quit ) then the passengers and train have to end without ride*/
        if(flag == 1){
            break;
        }
        
        
        if(select == 'p') {
            printf("%c is on the train and its' id is %lu\n",select,pthread_self());
            
        }
        
        /*==========TRAIN_RIDE===========*/
        sleep(1);
        /*===============================*/
        
        if(select == 't') {
            
            printf("train is waiting for passengers to leave, train id is : %lu\n",pthread_self());
            printf("\n");
            
            /*train is waiting for passengers to leave*/
            
            /*============================================================*/
            CCR_EXEC(train_wait, (val_train_wait == 1), val_train_wait = 1)
            /*============================================================*/
            
            first_time = 0;
            
        }
        
        if((flag == 1) && (select == 't')){
            break;
        }
        
        if(select == 'p') {
            /*=======================================================*/
            CCR_EXEC(train_wait, (1), val_train_wait = 1)
            /*=======================================================*/
            break;
        }
    }
}



