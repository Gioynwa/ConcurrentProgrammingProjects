#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



typedef struct bsemaphore {
	
	int value;
	pthread_mutex_t locker;
	
}mybsem;


int mybsem_init(mybsem *sem, int number);

int mybsem_destroy(mybsem *sem);

int mybsem_down(mybsem *sem);

int mybsem_up(mybsem *sem);
