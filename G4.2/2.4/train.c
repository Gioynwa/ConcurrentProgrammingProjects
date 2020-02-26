#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "mybsem.h"

volatile int max_passengers, nofpass = 0,passengers_out = 0,flag = 0,first_time = 1,counter = 0;
volatile mybsem start,train_wait,rest_passengers,done,train_finished;


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
    
    mybsem_init((mybsem*)&start, 0);
    mybsem_init((mybsem*)&train_wait,0);
    mybsem_init((mybsem*)&rest_passengers,0);
    mybsem_init((mybsem*)&done,0);
    mybsem_init((mybsem*)&train_finished,0);
    
    printf("Press 't' for train, 'p' for passenger or 'q' for exit.\n");
    
    while(1) {
        
        sleep(1);
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
            /*wake up the train in order to end wherever it is*/
            mybsem_up((mybsem*)&start);
            mybsem_up((mybsem*)&train_wait);
            mybsem_up((mybsem*)&done);
            
            
            break;
        }
    }
    
    mybsem_down((mybsem*)&train_finished);
    
    mybsem_destroy((mybsem*)&start);
    mybsem_destroy((mybsem*)&train_wait);
    mybsem_destroy((mybsem*)&rest_passengers);
    mybsem_destroy((mybsem*)&done);
    mybsem_destroy((mybsem*)&train_finished);
    
    return(0);
    
}


void *train_ride(char *c) {
    
    char select;
    int i;
    
    select = *c;
    
    /*in case that a lot of passengers come at one time*/
    if(select == 'p'){
        counter++;
        if(counter > max_passengers) {
            //the rest passengers have to wait their time
            mybsem_down((mybsem*)&rest_passengers);
            mybsem_up((mybsem*)&done);
            
        }
    }
    
    while(1){
        if(select == 'p') {
            nofpass++;
            /*when the last passenger come each time the train starts*/
            if(nofpass%max_passengers == 0) {
                mybsem_up((mybsem*)&start);
            }
        }
        
        if(select == 't'){
            printf("train is waiting for passengers to come,train id is : %lu\n",pthread_self());
            mybsem_init((mybsem*)&start,0);
            mybsem_init((mybsem*)&train_wait,0);
        }
        
        if((first_time == 0) && (select == 't')){
            /*if other passengers wait the train takes them the right time*/
            for(i = 0;i < max_passengers; i++){
                mybsem_up((mybsem*)&rest_passengers);
                mybsem_down((mybsem*)&done);
                if(flag == 1){
                    break;
                }
            }
        }
        
        /*train and passengers must be together*/
        mybsem_down((mybsem*)&start);
        mybsem_up((mybsem*)&start);
        
        /*if selection is q ( quit ) then the passengers and train have to end without ride*/
        if(flag == 1){
            break;
        }
        
        
        if(select == 'p') {
            printf("%c is on the train and its' id is %lu\n",select,pthread_self());
            
        }
        
        sleep(1);
        
        if(select == 't') {
            printf("train is waiting for passengers to leave, train id is : %lu\n",pthread_self());
            printf("\n");
            sleep(1);
            /*train is waiting for passengers to leave*/
            mybsem_down((mybsem*)&train_wait);
            first_time = 0;
        }
        
        if((flag == 1) && (select == 't')){
            break;
        }
        
        
        if(select == 'p') {
            passengers_out++;
            if(passengers_out%max_passengers == 0){
                /*the last passenger notify train*/
                mybsem_up((mybsem*)&train_wait);
            }
            break;
        }
    }
    if(select == 't'){
        mybsem_up((mybsem*)&train_finished);
    }
}





