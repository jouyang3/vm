

#include "VirtualMachine.h"
#include "scheduler.h"
#include "tcb.h"
#include "Machine.h"
#include <queue>
#include <stdlib.h>

extern TCB* currentThread;
extern std::priority_queue<TCB*> readyQueue;
extern void idle_stub(void* param);

void popThread();

void schedule()
{
//	TMachineSignalState sigstate;
//	MachineSuspendSignals(&sigstate);
	
	//p("TCB id: %d\n", tcb->tid);
	
	//p("TCB state: %d\n", tcb->state);
    
    p("[scheduler.cpp schedule()] currentThread->tid = %d\n",currentThread->tid);
	p("[scheduler.cpp schedule()] currentThread->state = %d\n",currentThread->state);
	if(currentThread->state == VM_THREAD_STATE_RUNNING)
	{
		if(currentThread->quantum == 0 || currentThread->priority == 0)
		{
			currentThread->state = VM_THREAD_STATE_READY;
            readyQueue.push(currentThread);
            
            popThread();
		}
	} 
        else if(currentThread->state == VM_THREAD_STATE_DEAD)
    	{	
		popThread();
    	} 
    	else if(currentThread->state == VM_THREAD_STATE_WAITING)
    	{
        	if(currentThread->nikita == VM_TIMEOUT_IMMEDIATE)
        	{
            		currentThread->state = VM_THREAD_STATE_READY;
            		readyQueue.push(currentThread);
            
           		TCB* oldThread = currentThread;
            		popThread();
           		 currentThread->quantum += oldThread->quantum;
       		 }
       		 else if(currentThread->nikita > 0)
       		 {
           		 popThread();
       		 }
   	}
//	MachineResumeSignals(&sigstate);
}

void popThread()
{
    if(!readyQueue.empty())
    {
        TCB* oldThread = currentThread;
        currentThread = readyQueue.top();
        readyQueue.pop();
	if(currentThread->nikita == 0)
        	currentThread->state = VM_THREAD_STATE_RUNNING;
        //MachineContextSave(oldThread->context);
        p("tid: %d -> tid: %d\n", oldThread->tid, currentThread->tid);
        MachineContextSwitch(oldThread->context, currentThread->context);
    }
}

