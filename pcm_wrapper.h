#ifndef __TSXWRAPPER_H__
#define __TSXWRAPPER_H__ 
#ifdef __cplusplus
extern "C" {
#endif
    /* PCM related */
    void* PCM_create(void);
    void api_get_CPUModel(void*);
    void PCM_program(void*, void*);
    __uint32_t PCM_getNumCores(void*);
    void* PCM_getAllCounterStates(void*);
    void* PCM_getCoreCounterState(void*, int);
    __uint64_t PCM_getNumberofCustomEvents(void*, void*, int, int);
    void PCM_delete(void*);
    void CoreState_delete(void*);

    /* PCM internal */
    
    /* ExtendedCustomCoreEventDescription */
    void* tsx_init_ExtendedCustomCoreEventDescription(void);
    void print_conf(void*);

#ifdef __cplusplus
}
#endif

#define N_RTM_RETIRED_START (0)
#define N_RTM_RETIRED_COMMIT (1)
#define N_RTM_RETIRED_ABORTED (2)


static inline int set_cpu(int);

#endif

