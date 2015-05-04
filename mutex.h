#ifndef MUTEX_H
#define MUTEX_H

#include <list>
#include "VirtualMachine.h"
#include "tcb.h"

#ifdef __cplusplus
extern "C" {
#endif

class TCB;

enum mutexState {UNLOCKED, LOCKED};

class Mutex
{
    public:
        Mutex(void);
        
        static unsigned int inc_mid;
		TVMMutexID mid;
		TVMMutexIDRef midRef;
		mutexState state;
		TCB* owner;
		std::list<TCB*> waitingLow;
		std::list<TCB*> waitingNormal;
		std::list<TCB*> waitingHigh;
};

#ifdef __cplusplus
}
#endif


#endif
