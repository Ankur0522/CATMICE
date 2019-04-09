#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#define NO_BOWL    2      
#define NO_CATS      3     
#define NO_MICE      2     
#define C_WAIT    10     
#define C_EAT        1       
#define C_N_EAT      3       
#define M_WAIT     1       
#define M_EAT      1       
#define M_N_EAT    3       

typedef struct bowl

 {
    int free_dishes;            
    int cats_eating;           
    int mice_eating;           
    int cats_waiting;        
    enum 
	{
        none_eating,
        cat_eating,
        mouse_eating
    } 
	status[NO_BOWL];         
    pthread_mutex_t mutex;      
    pthread_cond_t free_cv;     
    pthread_cond_t cat_cv;     
} bowl_t;
