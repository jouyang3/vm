#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "VirtualMachine.h"
#include "VirtualMachineUtils.h"
#include "tcb.h"

#ifdef __cplusplus
extern "C" {
#endif
        
    void schedule(void);
    
#ifdef __cplusplus
}
#endif

#endif
