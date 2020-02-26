#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

volatile int *number;
volatile int stop = 0,flag = 1;
volatile int *plus,mikos;

void *run_threads(void *info);

struct sort {
    int begin;
    int end;
    int k;
};

void quicksort(int first, int last,int k){
   
    int i, j, pivot, temp, check, mikos_left, mikos_right;
    pthread_t thread_left, thread_right;
    struct sort left , right;
    int l;
    
    l = k;
    
    if(first<last){
        pivot=first;
        i=first;
        j=last;

        while(i<j){
            while(number[i]<=number[pivot]&&i<last)
                i++;
            while(number[j]>number[pivot])
                j--;
            if(i<j){
                temp=number[i];
                number[i]=number[j];
                number[j]=temp;
            }
        }

        temp=number[pivot];
        number[pivot]=number[j];
        number[j]=temp;
         
        if(last - first + 1 <= 2){
            plus[l] = 0;
            
        }
        else{
            
            left.begin = first;
            left.end = j-1;
            
            right.begin = j + 1;
            right.end = last;
            
            check = pthread_create(&thread_left, NULL, run_threads,(void*)&left);
            if(check != 0){
                printf("error thread_left\n");
                exit(1);
            }  
            
            check = pthread_create(&thread_right, NULL, run_threads,(void*)&right);
                    
            if(check != 0){
                printf("error thread_right\n");
                exit(1);
            } 
            
        }
      
    } 
    
}

void *run_threads(void *info) {
    
    struct sort info1;
    int n;
    
    info1 = *((struct sort*)info);
    
    if(flag == 1){//if it is the main thread (for the malloc)
        info1.k = 0;
        mikos = 0;
        plus = (int*)malloc(3*sizeof(int));
        flag = 0;
        plus[0] = 1;// we put one that means the thread haven't finished yet
        plus[1] = 1;
        plus[2] = 1;
    }
    else{
        mikos++;
        info1.k = mikos;
        if(info1.end - info1.begin + 1 >=2){
            plus = (int*)realloc((int*)plus,3*(mikos+1)*sizeof(int));
            plus[2*(info1.k) + 1] = 1;
            plus[2*(info1.k) + 2] = 1; 
            
        }
        else{
            plus = (int*)realloc((int*)plus,(mikos+1)*sizeof(int));
            plus[info1.k] = 0;
        }
        
    }
    
    n = info1.k;
    quicksort(info1.begin,info1.end,info1.k);

    
    if(info1.end - info1.begin + 1 <= 2){
        if(n%2 == 0){
            while(plus[n-1] != 0){}//wait for the brother
            plus[n/2 - 1] = 0;//only if is the last thread
        }
        else{
            while(plus[n+1] !=0){}//wait for the sister
            plus[n/2] = 0;//only if is the last thread
        }
    }
    else{
        while(plus[2*n+1] == 1 || plus[2*n+2] == 1){}//wait for childs
        plus[n] = 0;//if the childs finished then the dad has to finish
    }

    if(n == 0){
        if(plus[0] == 0 ){//if the root of the tree returns main ends
            stop = 1;
        }
    }
    
}

int main(int argc, char *argv[]){
   
    struct sort main;
    int i, check, count;
    pthread_t thread_start;

    printf("How many elements are u going to enter?: ");
    scanf("%d",&count);

    printf("Enter %d elements: ", count);
    
    number = (int*)malloc(count*sizeof(int));
    if(number == NULL) {
        printf("memory allocation error\n");
    }
    
    for(i=0; i<count; i++) {
        scanf("%d",&number[i]);
    }
    
    main.begin = 0;//the mains' thread begin
    main.end = count - 1;//its' end
    
    check = pthread_create(&thread_start, NULL, run_threads, (void*)&main);
    
    if(check != 0){
        printf("error main thread\n");
        exit(1);
    } 
    
    while(stop == 0) {}

    printf("Order of Sorted elements: ");
    for(i=0;i<count;i++)
        printf(" %d",number[i]);
    
    free((int*)number);
    free((int*)plus);

    return 0;
}
