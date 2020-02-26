#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

volatile int max_passengers, nofpass = 0, flag = 0, first_time = 1, counter = 0;
volatile int val_start = 0, val_rest = 0, val_train_wait = 0, flag_in = 1, val_locker = 0;
volatile pthread_mutex_t mtx_rest, mtx_start, mtx_train_wait, mtx_locker; 
volatile pthread_cond_t rest, start, train_wait, locker;

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
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_rest, NULL) != 0) {
        perror("");
    }
    
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_start, NULL) != 0) {
        perror("");
    }
    
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_train_wait, NULL) != 0) {
        perror("");
    }
    
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_locker, NULL) != 0) {
        perror("");
    }
    
    if (pthread_cond_init((pthread_cond_t*)&rest, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }
    
    if (pthread_cond_init((pthread_cond_t*)&start, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }
    
    if (pthread_cond_init((pthread_cond_t*)&train_wait, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }
    
    if (pthread_cond_init((pthread_cond_t*)&locker, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }

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
    
    /*==============================DESTROY===================================*/
    pthread_mutex_destroy((pthread_mutex_t*)&mtx_rest); 
    
    pthread_mutex_destroy((pthread_mutex_t*)&mtx_start);
    
    pthread_mutex_destroy((pthread_mutex_t*)&mtx_train_wait);
        
    pthread_mutex_destroy((pthread_mutex_t*)&mtx_locker);
    
    pthread_cond_destroy((pthread_cond_t*)&rest); 
    
    pthread_cond_destroy((pthread_cond_t*)&start);
		
    pthread_cond_destroy((pthread_cond_t*)&train_wait);
	
	pthread_cond_destroy((pthread_cond_t*)&locker);

    /*============================================================================*/
    
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
				/*=========================MONITOR LOCKER(DOWN)=================================*/
				if(pthread_mutex_lock((pthread_mutex_t*)&mtx_locker) != 0){
					perror("");
				}
			
				while(val_locker == 0) {
					if (pthread_cond_wait((pthread_cond_t*)&locker, (pthread_mutex_t*)&mtx_locker) != 0) {                                  
						perror("pthread_cond_wait() error");                                   
					}
				}
				
				val_locker = 0;
				if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_locker) != 0){
					perror("");
				}
				 /*==========================================================================*/
				 
				/*=========================MONITOR LOCKER(UP)=================================*/ 
				if(pthread_mutex_lock((pthread_mutex_t*)&mtx_locker) != 0){
						perror("");
				}
				
				val_locker = 1;
                if (pthread_cond_signal((pthread_cond_t*)&locker) != 0) {                                  
                    perror("pthread_cond_signal() error");                                   
                }
                
				if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_locker) != 0){
						perror("");
				}
				/*==========================================================================*/
 			}
 			
            /*=========================MONITOR REST(DOWN)=================================*/
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
            
            while(val_rest == 0) {
                if (pthread_cond_wait((pthread_cond_t*)&rest, (pthread_mutex_t*)&mtx_rest) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
                }
            }
            val_rest = 0;
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
            /*=============================================================================*/
            
            /*=========================MONITOR REST(UP)=================================*/
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
                
            val_rest = 1;
            if (pthread_cond_signal((pthread_cond_t*)&rest) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
            /*===========================================================================*/
        }
    }
    
    while(1){
        if((select == 'p')) {
            nofpass++;
            /*when the last passenger come each time the train starts*/
            if(nofpass%max_passengers == 0) {
                /*=========================MONITOR START(UP)=================================*/
                if(pthread_mutex_lock((pthread_mutex_t*)&mtx_start) != 0){
                    perror("");
                }
            
                val_start = 1;
                if (pthread_cond_signal((pthread_cond_t*)&start) != 0) {                                  
                    perror("pthread_cond_signal() error");                                   
                }
            
                if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_start) != 0){
                    perror("");
                }
                /*===========================================================================*/
            }
        }
            
            
        if(select == 't'){
            printf("train is waiting for passengers to come,train id is : %lu\n",pthread_self());
        }
        
        if((first_time == 0) && (select == 't')) {
            /*if other passengers wait the train takes them the right time*/
			if(flag_in == 1) {
				flag_in = 0;
				/*=========================MONITOR LOCKER(UP)=================================*/ 
				if(pthread_mutex_lock((pthread_mutex_t*)&mtx_locker) != 0){
						perror("");
				}
				
				val_locker = 1;
                if (pthread_cond_signal((pthread_cond_t*)&locker) != 0) {                                  
                    perror("pthread_cond_signal() error");                                   
                }
                
				if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_locker) != 0){
						perror("");
				}
				/*========================================================================*/ 
			}

            /*=========================MONITOR REST(UP)=================================*/
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
            
            val_rest = 1;
        
            if (pthread_cond_signal((pthread_cond_t*)&rest) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
        
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_rest) != 0){
                perror("");
            }
            /*===========================================================================*/
        }
        
        /*train and passengers must be together*/
        /*=========================MONITOR START(DOWN)=================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&mtx_start) != 0){
            perror("");
        }
        
        while(val_start == 0) {
            if (pthread_cond_wait((pthread_cond_t*)&start, (pthread_mutex_t*)&mtx_start) != 0) {                                  
                perror("pthread_cond_wait() error");                                   
            }
        }
        val_start = 0;
            
        if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_start) != 0){
            perror("");
        }
        /*==============================================================================*/
		if((select == 't') && (first_time == 0) && (flag_in == 0)) {
			
			/*=========================MONITOR LOCKER(DOWN)=================================*/ 
			if(pthread_mutex_lock((pthread_mutex_t*)&mtx_locker) != 0){
				perror("");
			}
			
			while(val_locker == 0) {
                if (pthread_cond_wait((pthread_cond_t*)&locker, (pthread_mutex_t*)&mtx_locker) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
                }
            }
			val_locker = 0;
			if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_locker) != 0){
				perror("");
			}
			/*========================================================================*/ 
			flag_in = 1;
		}
        /*=========================MONITOR START(UP)====================================*/
        if(pthread_mutex_lock((pthread_mutex_t*)&mtx_start) != 0){
            perror("");
        }
        
        val_start = 1;
        if (pthread_cond_signal((pthread_cond_t*)&start) != 0) {                                  
            perror("pthread_cond_signal() error");                                   
        }
            
        if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_start) != 0){
            perror("");
        }
        /*==============================================================================*/
        
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
            
            /*=========================MONITOR TRAIN_WAIT(DOWN)=================================*/
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_train_wait) != 0){
                perror("");
            }
            
            while(val_train_wait == 0) {
                if(pthread_cond_wait((pthread_cond_t*)&train_wait, (pthread_mutex_t*)&mtx_train_wait) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
                }
            }
            val_train_wait = 0;
            val_start = 0;
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_train_wait) != 0){
                perror("");
            }
            /*===================================================================================*/
            
            first_time = 0;
            
        }
        
        if((flag == 1) && (select == 't')){
            break;
        }
        
        if(select == 'p') {
			/*=========================MONITOR TRAIN_WAIT(UP)=================================*/
			if(pthread_mutex_lock((pthread_mutex_t*)&mtx_train_wait) != 0){
				perror("");
			}
			
			val_train_wait = 1;
			
			if (pthread_cond_signal((pthread_cond_t*)&train_wait) != 0) {                                  
				perror("pthread_cond_signal() error");                                   
			}
			
			if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_train_wait) != 0){
				perror("");
			}
			/*=================================================================================*/
            break;
        }
    }
}



