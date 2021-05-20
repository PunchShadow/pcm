#ifdef __cplusplus
#define EXTERNC extern "C"
    #include "cpucounters.h"
    #include "utils.h"
    #include "profile.h"
    #include "rtm.h"

    #include <iostream>
    #include <iomanip>
    #include <string.h>
    #include <assert.h>
    #include <unistd.h>
    #include <pthread.h>
#else
#define EXTERNC
#endif

/*
typedef void* PCM_t;
typedef void* ErrorCode_t;
typedef void* ExtendedCustomCoreEventDescription_t;

EXTERNC ExtendedCustomCoreEventDescription_t  

EXTERNC PCM_t api_create_pcm(void);
EXTERNC void api_destory_pcm(void*);
EXTRENC ErrorCode_t api_program(void*, int EXT_CUSTOM_CORE_EVENTS, )
*/


EXTERNC void *monitor(void *arg);

#undef EXTERNC


typedef struct
{
    int thread_id;
    int sample_period; // second
} Monitor_arg;



// For pthread work
void *monitor(void *arg)
{
    Monitor_arg *data = (Monitor_arg *)arg;
    int thread_id = data->thread_id;
    int sample_period = data->sample_period;

    EventSelectRegister def_event_select_reg;
    def_event_select_reg.value = 0;
    def_event_select_reg.fields.usr = 1;
    def_event_select_reg.fields.os = 1;
    def_event_select_reg.fields.enable = 1;

    PCM::ExtendedCustomCoreEventDescription conf;
    conf.fixedCfg = NULL;
    EventSelectRegister regs[PERF_MAX_CUSTOM_COUNTERS];
    conf.gpCounterCfg = regs;
    for (int i=0; i < PERF_MAX_CUSTOM_COUNTERS; ++i)
        regs[i] = def_event_select_reg;
    
    regs[N_RTM_RETIRED_START].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_START].fields.umask = 0x01;
    regs[N_RTM_RETIRED_COMMIT].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_COMMIT].fields.umask = 0x02;
    regs[N_RTM_RETIRED_ABORTED].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_ABORTED].fields.umask = 0x04;

    conf.nGPCounters = 3;

    PCM * m = PCM::getInstance();
    PCM::ErrorCode resturnResult = m->program(PCM::EXT_CUSTOM_CORE_EVENTS, &conf);
    if (resturnResult != PCM::Success) {
        std::cerr << "Intel's PCM couldn't start" << std::endl;
        std::cerr << "Error code: " << resturnResult << std::endl;
        exit(1);
    }

    //print_cpu_details();

    /* Initializing */
    uint64 BeforeTime = 0, AfterTime = 0;
    SystemCounterState SysBeforeState, SysAfterState;
    const uint32 ncores = m->getNumCores();
    std::vector<CoreCounterState> BeforeState, AfterState;
    std::vector<SocketCounterState> DummySocketStates;


    BeforeTime = m->getTickCount();
    m->getAllCounterStates(SysBeforeState, DummySocketStates, BeforeState);

    
    while(1)
    {
        /* Sleeping, waiting for sampling */
        while(!sleep(sample_period)){}

        AfterTime = m->getTickCount();
        m->getAllCounterStates(SysAfterState, DummySocketStates, AfterState);
        
        cout << "Time elapsed: " << dec << fixed << AfterTime - BeforeTime << " ms\n";
        
        for (uint32 core = 0; core < ncores; ++core) 
        {
            cout << "Core " << core << " :";
            for(int k=0; k < 3; k++) {
            cout << getNumberOfCustomEvents(k, BeforeState[core], AfterState[core]) << ",";
            }
            cout << "\n";
        }
    }

    exit(EXIT_SUCCESS);
    pthread_exit((void *)0);
}


