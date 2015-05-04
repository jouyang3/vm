#include "mutex.h"
#include "utils.h"
//TVMMutexID mid;
//TVMMutexIDRef midRef;
//mutexState state;
//TCB* owner;
//std::list<TCB*> waitingLow;
//std::list<TCB*> waitingNormal;
//std::list<TCB*> waitingHigh;

Mutex::Mutex()
{
	p("[mutex.cpp Mutex::Mutex()]Attempt to create mutex with mid: %d\n", inc_mid);
	this->mid = inc_mid++;
	this->state = UNLOCKED;	
}
