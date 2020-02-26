#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "ccr.h"

/* WE SUPPOSE THAT THE BLUE AND RED CARS GO THROUGH THE BRIDGE ΑLTERNATELY*/


volatile int max_bridge_cars, val_red = 1, val_blue = 1;
volatile int counter = 0,first_time = 1,counter_blue = 0,counter_red = 0;

CCR_DECLARE(blue);
CCR_DECLARE(red);

void *cars(char *color);
void bridge_function(char c);
    

int main(int argc, char *argv[]) {
    
    int check, i,j;
    pthread_t thread_red,thread_blue;
    char color,red_color,blue_color;
    
    
    if(argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    
    max_bridge_cars = atoi(argv[1]);
    
    /*============INITIALIZATIONS=============*/
    
   CCR_INIT(blue)
   CCR_INIT(red)

    /*========================================*/

    red_color = 'r';
    blue_color = 'b';
    
    while(1) { 

        scanf(" %c",&color);
        
        if(color == 'r'){
            
            counter_red++;
            pthread_create(&thread_red, NULL, (void*)cars, (void*)&red_color);
            
        }
        if(color == 'b'){
            
            counter_blue++;
            pthread_create(&thread_blue, NULL, (void*)cars, (void*)&blue_color);
            
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
            
            /*=====================================================*/
            CCR_EXEC(red,(val_red == 0),val_red = 1)
            /*=====================================================*/
            counter++;
        }
        
    }
    
    if (select == 'b'){
        
        if((first_time == 1) && (counter == 0)){
            
            counter++;
            first_time = 0;
        }
        else{
            
            /*=============================================================*/
            CCR_EXEC(blue,(val_blue == 0),val_blue = 1)
            /*==============================================================*/
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
            
            /*==========================================================*/
            CCR_EXEC(red,(1),(val_red = 0))
            /*==========================================================*/
        }
    }
    
    if((counter <= max_bridge_cars) && (select == 'b')){
        
        if(counter != max_bridge_cars){
            
            /*=========================================================*/
            CCR_EXEC(blue,(1),(val_blue = 0))
            /*=========================================================*/
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
            
            /*=======================================================*/
            CCR_EXEC(blue,(1),(val_blue = 0))
            /*=======================================================*/
        }
        else if ((counter_red != 0 ) && (counter == 0)){
            
            /*========================================================*/
            CCR_EXEC(red,(1),(val_red = 0))
            /*========================================================*/
        }
        else{
            if(counter == 0){
                first_time = 1;
                
                /*=====================================================*/
                CCR_EXEC(red,(val_red == 0),val_red = 1)
                /*=====================================================*/
            }
        }
    }
    
    if(select == 'b') {
        
        if(counter != 0){
            counter--;
        }
        
        if((counter_red != 0) && (counter == 0)){
            
            /*====================================================*/
            CCR_EXEC(red,(1),(val_red = 0))
            /*====================================================*/
            
        }
        else if ((counter_blue != 0 ) && (counter == 0)){
            
            /*====================================================*/
            CCR_EXEC(blue,(1),(val_blue = 0))
            /*====================================================*/
            
        }
        else{
            if(counter == 0){
                first_time = 1;
                
                /*======================================================*/
                CCR_EXEC(blue,(val_blue == 0),(val_blue = 1))
                /*======================================================*/
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
    
