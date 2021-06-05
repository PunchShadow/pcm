#include "cpucounters.h"
#include "pcm_wrapper.h"



using namespace pcm;

void* PCM_create(void)
{
    return PCM::getInstance();
}

void PCM_delete(void* pcm)
{
    delete static_cast<PCM*>(pcm);
}

void CoreState_delete(void *corestate)
{
    static_cast<std::vector<CoreCounterState>*>(corestate)->clear();
}

void api_get_CPUModel(void* thiz)
{
    static_cast<PCM*>(thiz)->getCPUModel();
}

void PCM_program(void* pcm, void* conf)
{
    
    std::cout << "Execute program" << std::endl;

    /* Cast pcm to PCM object pointer and use PCM::program()*/
    /* conf is another Object, ExtendedCustomCoreEventDescription. 
       Since conf is passed through a pointer, it does not need to be casted.
    */
    PCM::ErrorCode resturnResult = 
    static_cast<PCM*>(pcm)->program(PCM::EXT_CUSTOM_CORE_EVENTS, conf);
    if (resturnResult != PCM::Success) {
        std::cerr << "Intel's PCM couldn't start" << std::endl;
        std::cerr << "Error code: " << resturnResult << std::endl;
        exit(1);
    }
}

void* PCM_getAllCounterStates(void* pcm)
{
    SystemCounterState SysState;
    std::vector<SocketCounterState> DummySocketStates;
    std::vector<CoreCounterState> *CoreState = new std::vector<CoreCounterState>;

    
    PCM *m = static_cast<PCM*>(pcm);
    m->getAllCounterStates(SysState, DummySocketStates, *CoreState);

    return CoreState;

}

uint64_t PCM_getNumberofCustomEvents(void* BeforeState, void* AfterState, int nb_event, int num_cores)
{
    // Cast to vector
    auto *before = static_cast<std::vector<CoreCounterState>*>(BeforeState);
    auto *after = static_cast<std::vector<CoreCounterState>*>(AfterState);
    return getNumberOfCustomEvents(nb_event, (*before)[num_cores], (*after)[num_cores]);
}


void* tsx_init_ExtendedCustomCoreEventDescription(void)
{
    EventSelectRegister def_event_select_reg;
    def_event_select_reg.value = 0;
    def_event_select_reg.fields.usr = 1;
    def_event_select_reg.fields.os = 1;
    def_event_select_reg.fields.enable = 1;

    PCM::ExtendedCustomCoreEventDescription* conf = new PCM::ExtendedCustomCoreEventDescription();
    conf->fixedCfg = NULL;
    EventSelectRegister* regs = new EventSelectRegister[PERF_MAX_CUSTOM_COUNTERS];
    

    for (int i=0; i < PERF_MAX_CUSTOM_COUNTERS; ++i)
        regs[i] = def_event_select_reg;
    
    regs[N_RTM_RETIRED_START].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_START].fields.umask = 0x01;
    regs[N_RTM_RETIRED_COMMIT].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_COMMIT].fields.umask = 0x02;
    regs[N_RTM_RETIRED_ABORTED].fields.event_select = 0xc9;
    regs[N_RTM_RETIRED_ABORTED].fields.umask = 0x04;

    conf->nGPCounters = 3;
    conf->gpCounterCfg = regs;

    return conf;

}

void print_conf(void* conf)
{
    std::cout << "conf.nGPCounters: " << static_cast<PCM::ExtendedCustomCoreEventDescription*>(conf)->nGPCounters << std::endl;
    std::cout << "conf.nGPCounters: " << static_cast<PCM::ExtendedCustomCoreEventDescription*>(conf)->gpCounterCfg[N_RTM_RETIRED_START].fields.event_select << std::endl;


}


uint32_t PCM_getNumCores(void* pcm)
{
    return static_cast<PCM*>(pcm)->getNumCores();
}