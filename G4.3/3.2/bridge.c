#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

/* WE SUPPOSE THAT THE BLUE AND RED CARS GO THROUGH THE BRIDGE ΑLTERNATELY*/


volatile int max_bridge_cars, cred = 0,cblue = 0, val_red = 0, val_blue = 0;
volatile int counter = 0,first_time = 1,counter_blue = 0,counter_red = 0;

volatile pthread_mutex_t mtx_control_reds, mtx_control_blue,mtx_first;
volatile pthread_cond_t control_reds, control_blue;
void *cars(char *color);
void bridge_function(char c);
    

int main(int argc, char *argv[]) {
    
    int check, i,j;
    pthread_t thread_red,thread_blue;
    char color,red,blue;
    
    
    if(argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    
    max_bridge_cars = atoi(argv[1]);
    
    /*=========================INITIALIZATIONS=================================*/
    
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_control_blue, NULL) != 0) {
        perror("");
    }
    
    if(pthread_mutex_init((pthread_mutex_t*)&mtx_control_reds, NULL) != 0) {
        perror("");
    }
    
    if (pthread_cond_init((pthread_cond_t*)&control_blue, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }
    
    if (pthread_cond_init((pthread_cond_t*)&control_reds, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
    }

    /*=========================================================================*/
    
    red = 'r';
    blue = 'b';
    
    while(1) { 

        scanf(" %c",&color);

        if(color == 'r'){
            
            counter_red++;
            pthread_create(&thread_red, NULL, (void*)cars, (void*)&red);
            
        }
        if(color == 'b'){
            
            counter_blue++;
            pthread_create(&thread_blue, NULL, (void*)cars, (void*)&blue);
            
        }
    }   
    

    return(0);
}



void *cars(char *color) {
    
    char select = *color;
    /*=============================================*/
    /*****************ENTRANCE CODE*****************/
    
    if (select == 'r'){
       
        if((first_time == 1) && (counter == 0)){
            
            counter++;
            first_time = 0;
        }
        else{
            
            /*=========================MONITOR CONTROL_REDS(DOWN)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            while(val_red == 0) {
                if (pthread_cond_wait((pthread_cond_t*)&control_reds, (pthread_mutex_t*)&mtx_control_reds) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
                }
            }
            val_red = 0;
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
             /*==================================================================================*/
            counter++;
        }
        
    }
    
    if (select == 'b'){
        
        if((first_time == 1) && (counter == 0)){
            
            counter++;
            first_time = 0;
        }
        else{
            
            /*=========================MONITOR CONTROL_BLUE(DOWN)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            while(val_blue == 0) {
                if (pthread_cond_wait((pthread_cond_t*)&control_blue, (pthread_mutex_t*)&mtx_control_blue) != 0) {                                  
                    perror("pthread_cond_wait() error");                                   
                }
            }
            val_blue = 0;
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
            counter++;
        }
        
    }
    
    if(select == 'r'){
        
        printf("Red car entrance in the bridge with id : %lu \n",pthread_self());
    }
    
    if(select == 'b'){

        printf("Blue car entrance in the bridge with id : %lu \n",pthread_self());
    }
    
    if((counter <= max_bridge_cars) && (select == 'r')){
        
        if(counter != max_bridge_cars){
            
            
            /*=========================MONITOR CONTROL_REDS(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            val_red = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_reds) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        
    }
    
    if((counter <= max_bridge_cars) && (select == 'b')){
        
        if(counter != max_bridge_cars){
            
             /*=========================MONITOR CONTROL_BLUE(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            val_blue = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_blue) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        
    }
    
    /*==========================================*/
    /*********************CS*********************/
    
    bridge_function(select);
    
    /*=========================================*/
    /*****************EXIT CODE*****************/
    
    if(select == 'r'){
        
        printf("Red car exit from the bridge with id : %lu \n",pthread_self());
        counter_red--;
    }
    
    if(select == 'b'){
        
        printf("Blue car exit from the bridge with id : %lu \n",pthread_self());
        counter_blue--;
    }
    
   
    if(select == 'r') {
        
        if(counter != 0 ){
            counter--;
        }
        
        if((counter_blue != 0) && (counter == 0)){
            
            /*=========================MONITOR CONTROL_BLUE(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            val_blue = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_blue) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        else if ((counter_red != 0 ) && (counter == 0)){
            
            /*=========================MONITOR CONTROL_REDS(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            val_red = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_reds) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        else{
            if(counter == 0){
                first_time = 1;
                
                /*=========================MONITOR CONTROL_REDS(DOWN)=================================*/
                
                if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_reds) != 0){
                    perror("");
                }
                
                while(val_red == 0) {
                    if (pthread_cond_wait((pthread_cond_t*)&control_reds, (pthread_mutex_t*)&mtx_control_reds) != 0) {                                  
                        perror("pthread_cond_wait() error");                                   
                    }
                }
                val_red = 0;
                
                if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_reds) != 0){
                    perror("");
                }
                
                /*==================================================================================*/
            
            }
        }
    }
    
    if(select == 'b') {
        
        if(counter != 0){
            counter--;
        }
        
        if((counter_red != 0) && (counter == 0)){
            
            /*=========================MONITOR CONTROL_REDS(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            val_red = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_reds) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_reds) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        else if ((counter_blue != 0 ) && (counter == 0)){
            
            /*=========================MONITOR CONTROL_BLUE(UP)=================================*/
            
            if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            val_blue = 1;
            if (pthread_cond_signal((pthread_cond_t*)&control_blue) != 0) {                                  
                perror("pthread_cond_signal() error");                                   
            }
            
            if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_blue) != 0){
                perror("");
            }
            
            /*==================================================================================*/
            
        }
        else{
            if(counter == 0){
                
                first_time = 1;
                
                /*=========================MONITOR CONTROL_BLUE(DOWN)=================================*/
                
                if(pthread_mutex_lock((pthread_mutex_t*)&mtx_control_blue) != 0){
                    perror("");
                }
                
                while(val_blue == 0) {
                    if (pthread_cond_wait((pthread_cond_t*)&control_blue, (pthread_mutex_t*)&mtx_control_blue) != 0) {                                  
                        perror("pthread_cond_wait() error");                                   
                    }
                }
                val_blue = 0;
                
                if(pthread_mutex_unlock((pthread_mutex_t*)&mtx_control_blue) != 0){
                    perror("");
                }
                
                /*==================================================================================*/
                
            }
        }
        
    }
    
    /*=========================================*/
}
    
void bridge_function(char c) {
    
    if(c == 'r') {
        
        printf("red car is in the bridge with id\t");
        printf("%lu\n",pthread_self());
        sleep(2);
        
    }
    
    if(c == 'b') {
    
        printf("blue car is in the bridge with id\t");
        printf("%lu\n",pthread_self());
        sleep(2);
        
    }
    
}
    
