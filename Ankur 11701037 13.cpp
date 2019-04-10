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
static void* 
cat(void *arg)
{
    bowl_t *bowl = (bowl_t *) arg;
    int n = C_N_EAT;
    int my_bowl = -1;
    int i;

    for (n = C_N_EAT; n > 0; n--) {

        pthread_mutex_lock(&bowl->mutex);
    
        pthread_cond_broadcast(&bowl->cat_cv);
        bowl->cats_waiting++;
        while (bowl->free_dishes <= 0 || bowl->mice_eating > 0) {
            pthread_cond_wait(&bowl->free_cv, &bowl->mutex);
        }
        bowl->cats_waiting--;

        assert(bowl->free_dishes > 0);
        bowl->free_dishes--;
        assert(bowl->cats_eating < NO_CATS);
        bowl->cats_eating++;
        
        for (i = 0; i < NO_BOWL && bowl->status[i] != none_eating; i++) ;
        my_bowl = i;
        assert(bowl->status[my_bowl] == none_eating);
        bowl->status[my_bowl] = cat_eating;
        dump_bowl("CAT", pthread_self(), "STARTED", bowl, my_bowl);
        pthread_mutex_unlock(&bowl->mutex);

        sleep(C_EAT);
        
        pthread_mutex_lock(&bowl->mutex);
        assert(bowl->free_dishes < NO_BOWL);
        bowl->free_dishes++;
        assert(bowl->cats_eating > 0);
        bowl->cats_eating--;
        bowl->status[my_bowl] = none_eating;

        pthread_cond_broadcast(&bowl->free_cv);
        dump_bowl("CAT", pthread_self(), "FINISHED", bowl, my_bowl);
        pthread_mutex_unlock(&bowl->mutex);

        sleep(rand() % C_WAIT);
    }

    return NULL;
}
static void* 
mouse(void *arg)
{
    bowl_t *bowl = (bowl_t *) arg;
    int n = M_N_EAT;
    struct timespec ts;
    struct timeval tp;
    int my_bowl;
    int i;

    for (n = M_N_EAT; n > 0; n--) {

        pthread_mutex_lock(&bowl->mutex);
        while (bowl->free_dishes <= 0 || bowl->cats_eating > 0
               || bowl->cats_waiting > 0) {
            pthread_cond_wait(&bowl->free_cv, &bowl->mutex);
        }

        assert(bowl->free_dishes > 0);
        bowl->free_dishes--;
        assert(bowl->cats_eating == 0);
        assert(bowl->mice_eating < NO_MICE);
        bowl->mice_eating++;

        for (i = 0; i < NO_BOWL && bowl->status[i] != none_eating; i++) ;
        my_bowl = i;
        assert(bowl->status[my_bowl] == none_eating);
        bowl->status[my_bowl] = mouse_eating;
        dump_bowl("MICE", pthread_self(), "STARTED", bowl, my_bowl);
        pthread_mutex_unlock(&bowl->mutex);
        
        gettimeofday(&tp,NULL);
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += M_EAT;
        pthread_mutex_lock(&bowl->mutex);
        pthread_cond_timedwait(&bowl->cat_cv, &bowl->mutex, &ts);
        pthread_mutex_unlock(&bowl->mutex);
        
        pthread_mutex_lock(&bowl->mutex);
        assert(bowl->free_dishes < NO_BOWL);
        bowl->free_dishes++;
        assert(bowl->cats_eating == 0);
        assert(bowl->mice_eating > 0);
        bowl->mice_eating--;
        bowl->status[my_bowl]=none_eating;

        pthread_cond_broadcast(&bowl->free_cv);
        dump_bowl("MICE", pthread_self(), "FINISHED", bowl, my_bowl);
        pthread_mutex_unlock(&bowl->mutex);
        
        sleep(rand() % M_WAIT);
    }

    return NULL;
}


