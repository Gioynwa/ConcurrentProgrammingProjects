#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "mybsem.h"

/* WE SUPPOSE THAT THE BLUE AND RED CARS GO THROUGH THE BRIDGE ΑLTERNATELY*/


volatile int max_bridge_cars,counter_red = 0, counter_blue = 0, cred = 0,cblue = 0, blue_unblocked = 0, red_unblocked = 0;
volatile int global_blue = 0, global_red = 0, counter = 0, first_time_blue = 0, first_time_red = 0, first_time = 1;

volatile mybsem control_main, control_blue, control_reds;
void *cars(char *color);
void bridge_function(char color);
    

int main(int argc, char *argv[]) {
    
    int check, i,j;
    pthread_t thread_red,thread_blue;
    char color;
    
    
    if(argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    
    max_bridge_cars = atoi(argv[1]);
    
    mybsem_init((mybsem*)&control_main, 0);
    mybsem_init((mybsem*)&control_blue, 0);
    mybsem_init((mybsem*)&control_reds, 0);
   

    while(1) { 

        scanf(" %c",&color);
        
        if(color == 'e'){
            if(counter_red != 0) {
                mybsem_up((mybsem*)&control_reds);
                
            }
            
            sleep(3);
            
            if(counter_blue != 0){
                mybsem_up((mybsem*)&control_blue);
            }
            
            sleep(3);
            break;
        }
        
        if(color == 'r'){
            pthread_create(&thread_red, NULL, (void*)cars, (void*)&color);
            sleep(1);
        }
        if(color == 'b'){
            pthread_create(&thread_blue, NULL, (void*)cars, (void*)&color);
            sleep(1);
        }
        
        
    }   
    
    
    mybsem_destroy((mybsem*)&control_main);
    mybsem_destroy((mybsem*)&control_blue);
    mybsem_destroy((mybsem*)&control_reds);
    

    return(0);
}



void *cars(char *color) {
    
    char select = *color;
    /*=======================*/
    /******ENTRANCE CODE******/
    
    //here we change flags with the necessary values whichever is the first color of 
    //car in order to go through the bridge this color of cars and then the cars will go through the bridge alternately
    
    if((select == 'b') && (first_time == 1)) {
        first_time = 0;
        first_time_blue = 1;
        first_time_red = 0;
    }
    
    if((select == 'r') && (first_time == 1)) {
        first_time = 0;
        first_time_blue = 0;
        first_time_red = 1;
    }
    
    /*FOR BLUE CARS*/
    if(select == 'b') {
        counter_blue++;
        if(first_time_blue == 1) {
            //if the first car is color of blue then this code runs for one time at start
            if(counter_blue > max_bridge_cars) {
                mybsem_down((mybsem*)&control_blue);
                if(blue_unblocked < max_bridge_cars - 1) {
                    mybsem_up((mybsem*)&control_blue);
                    blue_unblocked++;    /*red_unblocked counts every time the necessary cars to go through the bridge*/
                }
                else {
                    first_time_blue = 0;
                    blue_unblocked = 0;
                    /*if counter_red is non zero then there aren't any red cars waiting to go through the bridge*/
                    /*counter_red needed in case of some red cars are waiting to go through the bridge in the end*/
                    counter_blue = 0;    
                }
            }
        }
        else {
            //this code runs all the times except the first time if is the first car color of blue
            mybsem_down((mybsem*)&control_blue);
            if(blue_unblocked < max_bridge_cars - 1) {
                    mybsem_up((mybsem*)&control_blue);
                    blue_unblocked++;   /*blue_unblocked counts every time the necessary cars to go through the bridge*/
            }
            else {
                    blue_unblocked = 0;
                    /*if counter_blue is non zero then there aren't any blue cars waiting to go through the bridge*/
                    /*counter_blue needed in case of some blue cars are waiting to go through the bridge in the end*/
                    counter_blue = 0;   
            }
        }
    }
	
	/*FOR RED CARS*/
    if(select == 'r') {
        counter_red++;
        if(first_time_red == 1) {
            //if the first car is color of red then this code runs for one time at start
            if(counter_red > max_bridge_cars) {
                mybsem_down((mybsem*)&control_reds);
                if(red_unblocked < max_bridge_cars - 1) {
                    mybsem_up((mybsem*)&control_reds);
                    red_unblocked++;    /*red_unblocked counts every time the necessary cars to go through the bridge*/
                }
                else {
                    first_time_red = 0;
                    red_unblocked = 0;
                    /*if counter_red is non zero then there aren't any red cars waiting to go through the bridge*/
                    /*counter_red needed in case of some red cars are waiting to go through the bridge in the end*/
                    counter_red = 0;    
                }
            }
        }
        else {
            //this code runs all the times except the first time if is the first car color of red
            mybsem_down((mybsem*)&control_reds);
            if(red_unblocked < max_bridge_cars - 1) {
                mybsem_up((mybsem*)&control_reds);
                red_unblocked++;
            }
            else {
                red_unblocked = 0;
                counter_red = 0;
            }
        }
        
    }
    
    /*=======================*/
    /*==========CS============*/
    
    bridge_function(select);
    
    /*=======================*/
    /********EXIT CODE********/
	
    if(select == 'r') {
        cred++;     /*cred counts the red cars which exit the bridge in order to notify the next blue cars*/
        if(cred == max_bridge_cars) {
            mybsem_up((mybsem*)&control_blue);
            cred = 0;
        }
    }
    
    if(select == 'b') {
        cblue++;    /*cblue counts the blue cars which exit the bridge in order to notify the next red cars*/
        if(cblue == max_bridge_cars) {
            mybsem_up((mybsem*)&control_reds);
            cblue = 0;
        }
    }
    
    /*=====================*/
}
    
void bridge_function(char color) {
    
    int check;
    
    counter++;
    if(counter == 1) {
        printf("ENTRANCE IN THE BRIDGE\n");
    }
    
    if(counter == max_bridge_cars) {
        counter = 0;
    }
    
    if(color == 'r') {
        printf("red car is in the bridge with id\t");
        printf("%lu\n",pthread_self());
        
    }
    else if(color == 'b') {
        printf("blue car is in the bridge with id\t");
        printf("%lu\n",pthread_self());
        
    }
    
    
    
}
    
