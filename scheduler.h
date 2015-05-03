#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "VirtualMachine.h"
#include "VirtualMachineUtils.h"
#include "tcb.h"
    
    void schedule(void);
    
#ifdef __cplusplus
}
#endif

#endif
