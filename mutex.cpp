#include "mutex.h"

//TVMMutexID mid;
//TVMMutexIDRef midRef;
//mutexState state;
//TCB* owner;
//std::list<TCB*> waitingLow;
//std::list<TCB*> waitingNormal;
//std::list<TCB*> waitingHigh;

Mutex::Mutex()
{
	this->mid = inc_mid++;
	this->midRef = &(this->mid);
	this->state = UNLOCKED;	
}
