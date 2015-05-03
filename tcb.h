#ifndef TCB_H
#define TCB_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "Machine.h"
#include "VirtualMachine.h"
    
#define QUANTUM_PER_THREAD ((TVMTick) 20)
    
#define QUANTUM_PER_THREAD ((TVMTick) 20)
    
    class TCB
    {
    public:
        TCB(TVMThreadEntry, void*, TVMMemorySize, void*, TVMThreadPriority,
            TVMThreadIDRef);
        
        static unsigned int inc_tid;
        TVMThreadID tid;
        TVMThreadState state;
        TVMThreadPriority priority;
        TVMThreadEntry entry;
        void *param;
        TVMMemorySize memsize;
        void* stackBase;
        SMachineContext context;
        TVMTick slp_ctr;
        TVMTick nikita;
        TVMTick quantum;
        int fileResult;
        //Mutex Info
    };
    
#ifdef __cplusplus
}
#endif

#endif
