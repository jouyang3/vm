#include "utils.h"
#include "mutex.h"
#include "tcb.h"
#include <list>
#include <map>

std::map<int, Mutex*> tb_mutex;
std::map<int, TCB*> tb_tcb;
TCB* idleThread;
std::list<TCB*> readyLow;
std::list<TCB*> readyNormal;
std::list<TCB*> readyHigh;
std::list<TCB*> sleepList;

void enqueue(TCB* tcb)
{
    if(tcb->priority == 0){
        idleThread = tcb;
        return;
    }
    switch(tcb->priority)
    {
        case VM_THREAD_PRIORITY_LOW:
            readyLow.push_back(tcb); break;
        case VM_THREAD_PRIORITY_NORMAL:
            readyNormal.push_back(tcb); break;
        case VM_THREAD_PRIORITY_HIGH:
            readyHigh.push_back(tcb); break;
    }
}

TCB* peek()
{
    TCB* front = NULL;
    if(!readyHigh.empty())
    {
        front = readyLow.front();
    }
    else if(!readyNormal.empty())
    {
        front = readyNormal.front();
    }
    else if(!readyLow.empty())
    {
        front = readyLow.front();
    }
    else
    {
        front = idleThread;
    }
    return front;
}

TCB* peekPrior(TVMThreadPriority p)
{
    TCB* front = NULL;
    switch(p)
    {
        case 0:
        {
            front = idleThread;
            break;
        }
        case VM_THREAD_PRIORITY_LOW:
        {
            if(!readyLow.empty())
            {
                front = readyLow.front();
            }
            break;
        }
        case VM_THREAD_PRIORITY_NORMAL:
        {
            if(!readyNormal.empty())
            {
                front = readyNormal.front();
            }
            break;
        }
        case VM_THREAD_PRIORITY_HIGH:
        {
            if(!readyHigh.empty())
            {
                front = readyHigh.front();
            }
            break;
        }
    }
    return front;
}

TCB* dequeue()
{
    TCB* front = NULL;
    if(!readyHigh.empty())
    {
        front = readyHigh.front();
        readyHigh.pop_front();
    }
    else if(!readyNormal.empty())
    {
        front = readyNormal.front();
        readyNormal.pop_front();
    }
    else if(!readyLow.empty())
    {
        front = readyLow.front();
        readyLow.pop_front();
    }
    else
    {
        front = idleThread;
        idleThread = NULL;
    }
    return front;
}

TCB* dequeuePrior(TVMThreadPriority p)
{
    TCB* front = NULL;
    switch(p)
    {
        case 0:
        {
            front = idleThread;
            idleThread = NULL;
            break;
        }
        case VM_THREAD_PRIORITY_LOW:
        {
            if(!readyLow.empty())
            {
                front = readyLow.front();
                readyLow.pop_front();
            }
            break;
        }
        case VM_THREAD_PRIORITY_NORMAL:
        {
            if(!readyNormal.empty())
            {
                front = readyNormal.front();
                readyNormal.pop_front();
            }
            break;
        }
        case VM_THREAD_PRIORITY_HIGH:
        {
            if(!readyHigh.empty())
            {
                front = readyHigh.front();
                readyHigh.pop_front();
            }
            break;
        }
    }
    return front;
}

void menqueue(TCB* tcb, Mutex* m)
{
	switch(tcb->priority)
    {
        case VM_THREAD_PRIORITY_LOW:
            (m->waitingLow).push_back(tcb); break;
        case VM_THREAD_PRIORITY_NORMAL:
            (m->waitingNormal).push_back(tcb); break;
        case VM_THREAD_PRIORITY_HIGH:
            (m->waitingHigh).push_back(tcb); break;
    }
}

TCB* mpeekPrior(TVMThreadPriority p, Mutex* m)
{
	TCB* front = NULL;
    switch(p)
    {
        case 0:
        case VM_THREAD_PRIORITY_LOW:
        {
            if(!(m->waitingLow).empty())
            {
                front = (m->waitingLow.front());
            }
        }
        case VM_THREAD_PRIORITY_NORMAL:
        {
            if(!(m->waitingNormal).empty())
            {
                front = (m->waitingNormal).front();
            }
        }
        case VM_THREAD_PRIORITY_HIGH:
        {
            if(!(m->waitingHigh).empty())
            {
                front = (m->waitingHigh).front();
            }
        }
    }
    return front;
}
    
TCB* mdequeuePrior(TVMThreadPriority p, Mutex* m)
{
	TCB* front = NULL;
    switch(p)
    {
        case 0:
        case VM_THREAD_PRIORITY_LOW:
        {
            if(!(m->waitingLow).empty())
            {
                front = (m->waitingLow).front();
                (m->waitingLow).pop_front();
            }
        }
        case VM_THREAD_PRIORITY_NORMAL:
        {
            if(!(m->waitingNormal).empty())
            {
                front = (m->waitingNormal).front();
                (m->waitingNormal).pop_front();
            }
        }
        case VM_THREAD_PRIORITY_HIGH:
        {
            if(!(m->waitingHigh).empty())
            {
                front = (m->waitingHigh).front();
                (m->waitingHigh).pop_front();
            }
        }
    }
    return front;
}

void mutexWaiterIncrement(Mutex* mutex)
{
	//p("In MutexWaiterIncrement Function\n");
	for(std::list<TCB*>::iterator it = (mutex->waitingHigh).begin(); it != (mutex->waitingHigh).end(); it++)
	{
		TCB* tcb = *it;
		if(tcb->nikita != VM_TIMEOUT_INFINITE)
		{
			tcb->slp_ctr++;
			
			if(tcb->slp_ctr >= tcb->nikita)
			{
				it = --(mutex->waitingHigh).erase(it);
				tcb->state = VM_THREAD_STATE_DEAD;
			}
		}
	}
	for(std::list<TCB*>::iterator it = (mutex->waitingNormal).begin(); it != (mutex->waitingNormal).end(); it++)
	{
		TCB* tcb = *it;
		if(tcb->nikita != VM_TIMEOUT_INFINITE)
		{
			tcb->slp_ctr++;
			
			if(tcb->slp_ctr >= tcb->nikita)
			{
				it = --(mutex->waitingHigh).erase(it);
				tcb->state = VM_THREAD_STATE_DEAD;
			}
		}
	}
	for(std::list<TCB*>::iterator it = (mutex->waitingNormal).begin(); it != (mutex->waitingNormal).end(); it++)
	{
		TCB* tcb = *it;
		if(tcb->nikita != VM_TIMEOUT_INFINITE)
		{
			tcb->slp_ctr++;
			
			if(tcb->slp_ctr >= tcb->nikita)
			{
				it = --(mutex->waitingHigh).erase(it);
				tcb->state = VM_THREAD_STATE_DEAD;
			}
		}
	}
}
