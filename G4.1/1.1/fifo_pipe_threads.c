#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define MAX_SIZE 10

volatile char *array_fifo;
volatile int in, out,size;


void pipe_close() {
    
    free((char*)array_fifo);
    in = -2;
}


void pipe_init(int size) {
    
    int i;
    
    array_fifo = (volatile char*)malloc(size);
    if(array_fifo == NULL) {
        write(1, "malloc NULL\n", 12);
        exit(1);
    }
    
    for(i=0; i < size; i++) {
        array_fifo[i] = '\0';
    }
    
    in = 0;
    out = 0;
    
}


void pipe_write(char c) {
    
    int byte_read;
    
    array_fifo[in] = c;
    in++;
    
    while(byte_read != 0) {    
        
        while(in == size) {
            
            if(out != 0) {
                in = 0;
                
            }
        }
        
        while( in < out){}//thread1 faster than thread2
        
        byte_read = read(STDIN_FILENO, &c, sizeof(char));
        if(byte_read == -1) {
            write(STDOUT_FILENO, "byte read error\n", 16);
            exit(1);
        }
        
        array_fifo[in] = c;
        if(byte_read != 0) {
            in++;
        }
       
       
    }
    while(in != out){}//the time thread1 finished ,has to wait thread2 to reach the characters which thread2 has to read
    pipe_close();
}


int pipe_read(char *c) {
    
    write(STDOUT_FILENO, c, sizeof(char));
    
    while(in != -2) {
        while(in == out) {}
        
        if(in == -2) {
            break;
        }
        
        *c = array_fifo[out];
        array_fifo[out] ='\0';
        
        write(STDOUT_FILENO, c, sizeof(char));
        
        
        if(out == size-1) {
            while( in == size ){}//the case where in = 10 and out = 9 two pointers at the end of the array
            out = 0;
        }
        else {
            out++;
        }
    }
    return(0);
    
}


void *run_thread_write() {
    
    char c;
    int byte_read;
 
    byte_read = read(STDIN_FILENO, &c, sizeof(char));
    if(byte_read == -1) {
        write(STDOUT_FILENO, "byte read error\n", 16);
        exit(1);
    }
        
    
    if(byte_read != 0) {
        
        pipe_write(c);
                
    }

        
}


void *run_thread_read() {
    
    char c;
    int number = 1;
    
   
    while(in == out) {}

    c = array_fifo[out];
    out++;
    number = pipe_read(&c);
    
}


int main(int argc, char *argv[]) {
    
    
    int check, byte_read;
    char c;
    pthread_t thread_read, thread_write;
   
    
    if(argc != 2) {
        write(1,"Wrong number of arguments\n",26*sizeof(char));
        return(0);
    }
    
    size = atoi(argv[1]);
    pipe_init(size);
    
    check = pthread_create(&thread_write,NULL,run_thread_write,NULL);
    if(check != 0){
        write(1,"error - pthread_create - thread_write\n",100*sizeof(char));
    }
    
    
    check = pthread_create(&thread_read,NULL, run_thread_read,NULL);
    if(check != 0){
        write(1,"error - pthread_create - thread_write\n",100*sizeof(char));
    }
    
    while(in != -2){};
    

    
    return(0);
    
}
    






    
