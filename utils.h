#ifndef UTILS_H
#define UTILS_H

#include "mutex.h"    
#include "tcb.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define p(...) //fprintf(stderr, __VA_ARGS__)
	
	void enqueue(TCB*);
    TCB* peek();
    TCB* peekPrior(TVMThreadPriority);
    TCB* dequeue();
    TCB* dequeuePrior(TVMThreadPriority);
    void menqueue(TCB*, Mutex*);
    TCB* mdequeuePrior(TVMThreadPriority, Mutex*);
    TCB* mpeekPrior(TVMThreadPriority, Mutex*);
    void mutexWaiterIncrement(Mutex*);
    
#ifdef __cplusplus
}
#endif

#endif
