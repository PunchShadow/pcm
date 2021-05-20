/**
* File:
*   wrapper_test.c
* Description:
*   Testing C++ wrapper at C with pthread
*/
#define _GNU_SOURCE
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>  

//#include "pcm_tsx_wrapper.h"
#include "profile.h"
#include "wrapper.h"

#define NUM_WORKER 4


pthread_t monitor_thread;
pthread_t worker_thread[NUM_WORKER];


typedef struct
{
    int thread_id;
    int tx_start;
    int nb_commit;
    int nb_abort;
} Worker_arg;

typedef struct
{
    int thread_id;
    int sample_period;
    __uint64_t sum_commit;
    __uint64_t sum_abort;
} Monitor_arg;


static inline int set_cpu(int i)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(i, &mask);

    printf("thread %lu, i=%d\n", pthread_self(), i);
    if( -1 == pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask))
    {
        return -1;
    }
    return 0;
}



void *monitor(void *arg)
{
    Monitor_arg *data = (Monitor_arg *)arg;
    int thread_id = data->thread_id;

    // Setting CPU
    if(set_cpu(thread_id))
    {
        printf("set CPU erro\n");
    }

    /* Initilization of PCM*/
    printf("Enter monitor function!!!\n");
    void* PCM = PCM_create();
    void* conf = tsx_init_ExtendedCustomCoreEventDescription();
    
    PCM_program(PCM, conf);
    void* before_CoreStates = PCM_getAllCounterStates(PCM);
    uint32_t ncores = PCM_getNumCores(PCM);
    
    __uint64_t sum_commit, sum_abort = 0;

    while(1)
    {
        sleep(5); // Waiting worker threads executing.
        void* after_CoreStates = PCM_getAllCounterStates(PCM);

        for (int core=0; core < ncores; core++)
        {
            printf("Core %d :", core);
            for (int nb_event = 0; nb_event < 3; nb_event++)
            {
                printf("%ld, ", PCM_getNumberofCustomEvents(before_CoreStates, after_CoreStates, nb_event, core));
            }
            printf("\n");
        }
        sleep(2);
    }
    pthread_exit((void *)0);
}



/* Acting like RTM accesses */
void *worker(void *arg)
{
    Worker_arg *data = (Worker_arg *)arg;
    int thread_id = data->thread_id;
    int times = data->tx_start;
    int nb_commit, nb_abort = 0;

    // Setting CPU
    if(set_cpu(thread_id))
    {
        printf("set CPU erro\n");
    }
    sleep(2);
    // Randomly execute commit and abort
    for (int i=0; i < times; ++i)
    {
        srand(time(NULL));
        int x = rand()%(thread_id +1);
        if ( x%2 == 1){
            PROF_COMMIT();
            ++nb_commit;
        }
        else{
            PROF_ABORT();
            ++nb_abort;
        }
    }
    
    data->nb_abort = nb_abort;
    data->nb_commit = nb_commit;
    printf("Thread No.%d: %d, %d\n", thread_id, nb_commit, nb_abort);
    pthread_exit((void *)0);
}



int main(int argc, char *argv[])
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    Worker_arg w_arg[NUM_WORKER];
    Monitor_arg m_arg;
    
    /* Monitor thread create */
    m_arg.thread_id = NUM_WORKER;
    m_arg.sample_period = 1;
    m_arg.sum_abort = 0;
    m_arg.sum_commit = 0;

    pthread_create(&monitor_thread, &attr, monitor, (void *)&m_arg); 

    for (int i = 0; i < NUM_WORKER; i++)
    {
        w_arg[i].thread_id = i;
        w_arg[i].tx_start = 100000;
        w_arg[i].nb_abort = 0;
        w_arg[i].nb_commit = 0;
        pthread_create(&worker_thread[i], &attr, worker, (void *)&w_arg[i]);
    }

    pthread_attr_destroy(&attr);

    void *status;
    for (int i = 0; i < NUM_WORKER; i++)
    {
        pthread_join(worker_thread[i], &status);
    }
    pthread_join(monitor_thread, &status);

    pthread_exit(NULL);
    return 0;

}


