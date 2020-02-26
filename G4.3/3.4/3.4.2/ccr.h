#include <pthread.h>
#include <errno.h>

struct region {
    pthread_cond_t R_q;
    pthread_mutex_t R_mtx;
    pthread_mutex_t mtx_R_q;
    int R_n;
    int val;
};


#define CCR_DECLARE(label)\
struct region label;


#define CCR_INIT(label)\
if(pthread_mutex_init((pthread_mutex_t*)&label.R_mtx, NULL) != 0) {\
        perror("");\
}\
if(pthread_mutex_init((pthread_mutex_t*)&label.mtx_R_q, NULL) != 0) {\
        perror("");\
}\
if (pthread_cond_init((pthread_cond_t*)&label.R_q, NULL) != 0) {\
    perror("pthread_cond_init() error");\
}\
label.R_n = 0;\
label.val = 0;


#define CCR_EXEC(label,cond,body)\
if(pthread_mutex_lock((pthread_mutex_t*)&label.R_mtx) != 0){\
	perror("");\
}\
while(!cond) {\
	label.R_n++;\
	if(pthread_mutex_unlock((pthread_mutex_t*)&label.R_mtx) != 0){\
		perror("");\
	}\
	if(pthread_mutex_lock((pthread_mutex_t*)&label.mtx_R_q) != 0){\
		perror("");\
	}\
	while(label.val == 0){\
        if (pthread_cond_wait((pthread_cond_t*)&label.R_q, (pthread_mutex_t*)&label.mtx_R_q) != 0) {\
            perror("pthread_cond_waitl() error");\
        }\
    }\
	label.val = 0;\
    if(pthread_mutex_unlock((pthread_mutex_t*)&label.mtx_R_q) != 0){\
        perror("");\
    }\
}\
body;\
if(label.R_n > 0) {\
	label.R_n--;\
	if(pthread_mutex_lock((pthread_mutex_t*)&label.mtx_R_q) != 0){\
		perror("");\
	}\
	label.val = 1;\
	if (pthread_cond_signal((pthread_cond_t*)&label.R_q) != 0) {\
        perror("pthread_cond_signal() error");\
    }\
    if(pthread_mutex_unlock((pthread_mutex_t*)&label.mtx_R_q) != 0){\
        perror("");\
    }\
}\
else {\
    if(pthread_mutex_unlock((pthread_mutex_t*)&label.R_mtx) != 0){\
        perror("");\
    }\
}
