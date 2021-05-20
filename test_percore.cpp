#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "cpucounters.h"
#include "utiles.h"
#include "profile.h"

#define NUM_THREAD 8


typedef struct
{
    int thread_id;
    int exe_time;
    double *sum_rtm_start;
    double *sum_rtm_commit;
    double *sum_rtm_abort;
} Arg;

pthread_t callThd[NUM_THREAD];
pthread_mutex_t mutexsum;


void *rtm_execute(void *arg)
{
    Arg *data = (Arg *) arg;
    int thread_id = data->thread_id;
    int times = data->exe_time;
    
    // Randomly execute commit and abort
    for (int i=0; i < times; ++i)
    {
        srand(time(NULL));
        int x = rand();
        if ( x%2 == 1) PROF_COMMIT();
        else PROF_ABORT();
    }

    
    
    
}

void *rtm_calculate_all(void *arg)
{

}



int main(int argc, char *argv[])
{
    
    
}