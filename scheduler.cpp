

#include "VirtualMachine.h"
#include "scheduler.h"
#include "tcb.h"
#include "Machine.h"
#include <queue>
#include <stdlib.h>


volatile TVMTick quantum;
TCB* runningThread;
extern std::priority_queue<TCB*> readyQueue;

void schedule()
{
	TMachineSignalState sigstate;
	MachineSuspendSignals(&sigstate);
	
	TCB* tcb = readyQueue.top();
	//p("TCB id: %d\n", tcb->tid);
	
	readyQueue.pop();
	tcb->state = VM_THREAD_STATE_RUNNING;
	//p("TCB state: %d\n", tcb->state);
	/*
	if(runningThread->state == VM_THREAD_STATE_RUNNING)
	{
		if(quantum == 0)
		{
			runningThread->state = VM_THREAD_STATE_READY;
			
		}
	}
	*/
	MachineResumeSignals(&sigstate);
}

/*
void IdleThread(void*)
{
	while(1);
}
*/
