#ifndef VIRTUALMACHINEUTILS_H
#define VIRTUALMACHINEUTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
    //#include <queue>
#include "VirtualMachine.h"
    
#include "tcb.h"
    
    
#define p(...) //fprintf(stderr, __VA_ARGS__)
    
    TVMMainEntry VMLoadModule(const char *module);
    void VMUnloadModule(void);
    TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);
    
    void enqueue(TCB*);
    TCB* peek();
    TCB* peekPrior(TVMThreadPriority p);
    TCB* dequeue();
    TCB* dequeuePrior(TVMThreadPriority);
    
#ifdef __cplusplus
}
#endif

#endif
