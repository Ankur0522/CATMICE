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

static const char *progname = "pets";

static void
dump_bowl(const char *name, pthread_t pet, const char *what,
          bowl_t *bowl, int my_bowl)
{
    int i;
    struct tm t;
    time_t tt;
    
    tt = time(NULL);
    assert(tt != (time_t) -1);
    localtime_r(&tt, &t);

    printf("\n\t\tTIME IS = %02d:%02d:%02d [", t.tm_hour, t.tm_min, t.tm_sec);
    for (i = 0; i < NO_BOWL; i++) {
        if (i) printf(":");
        switch (bowl->status[i]) {
        case none_eating:
            printf("-");
            break;
        case cat_eating:
            printf("C");
            break;
        case mouse_eating:
            printf("M");
            break;
        }
    }
    printf("]\n\n\t %s (id %x) %s EATING FROM BOWL :-  %d\n\n\n\n", name, pet, what, my_bowl);
}

