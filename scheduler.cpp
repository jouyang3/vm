

#include "VirtualMachine.h"
#include "scheduler.h"
#include "tcb.h"
#include "Machine.h"
#include <list>
#include <queue>
#include <stdlib.h>

extern TCB* currentThread;
extern std::list<TCB*> readyLow;
extern std::list<TCB*> readyMid;
extern std::list<TCB*> readyHigh;
extern void idle_stub(void* param);

void popThread();

void schedule()
{
    	TMachineSignalState sigstate;
	MachineSuspendSignals(&sigstate);
        p("[scheduler.cpp schedule()] currentThread->tid = %d\n",currentThread->tid);
	p("[scheduler.cpp schedule()] currentThread->state = %d\n",currentThread->state);
	if(currentThread->state == VM_THREAD_STATE_RUNNING)
	{
		if(currentThread->quantum == 0 || currentThread->priority == 0)
		{
			currentThread->state = VM_THREAD_STATE_READY;
            		enqueue(currentThread);
            		MachineResumeSignals(&sigstate);
            		popThread();
		}
		else
			MachineResumeSignals(&sigstate);
	} 
        else if(currentThread->state == VM_THREAD_STATE_DEAD)
    	{	
		MachineResumeSignals(&sigstate);
		popThread();
    	} 
    	else if(currentThread->state == VM_THREAD_STATE_WAITING)
    	{
        	if(currentThread->nikita == VM_TIMEOUT_IMMEDIATE)
        	{
			TCB* oldThread = currentThread;
            		oldThread->state = VM_THREAD_STATE_READY;
            		enqueue(oldThread);

            		currentThread = dequeuePrior(oldThread->priority);
			if(currentThread!=NULL)
			{
           			currentThread->quantum += oldThread->quantum;
				oldThread->quantum = QUANTUM_PER_THREAD;
				MachineResumeSignals(&sigstate);
				MachineContextSwitch(&(oldThread->context), &(currentThread->context));
			}
			else 
			{	
				MachineResumeSignals(&sigstate);
				oldThread->quantum = QUANTUM_PER_THREAD;
			}
       		 }
       		 else if(currentThread->nikita > 0)
       		 {
			MachineResumeSignals(&sigstate);
           		popThread();
       		 } else { // IO Operations
			MachineResumeSignals(&sigstate);
			popThread();
		}
   	}
}

void popThread()
{
	TCB* oldThread = currentThread;
	p("[scheduler.cpp popThread()] oldThread = #%d\n",oldThread->tid);
        currentThread = dequeue();
	if(currentThread->nikita == 0)
        	currentThread->state = VM_THREAD_STATE_RUNNING;
        //MachineContextSave(oldThread->context);
        p("[scheduler.cpp popThread()] tid: %d -> tid: %d\n", oldThread->tid, currentThread->tid);
	//p("#%d.context = %d\n",oldThread->tid, oldThread->context);
	//p("#%d.context = %d\n", currentThread->tid, currentThread->context);
        MachineContextSwitch(&(oldThread->context), &(currentThread->context));
}

