#ifndef TCB_H
#define TCB_H

#include <list>
#include "Machine.h"
#include "VirtualMachine.h"
#include "mutex.h"
    
#ifdef __cplusplus
extern "C" {
#endif

#define QUANTUM_PER_THREAD ((TVMTick) 20)
    
#define QUANTUM_PER_THREAD ((TVMTick) 20)

class Mutex;

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
        std::list<Mutex*> heldMutexes;
        Mutex* waitingOnMutex;
};
    

#ifdef __cplusplus
}
#endif


#endif
