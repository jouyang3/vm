#include <vector>
#include <queue>
#include <list>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "VirtualMachine.h"
#include "VirtualMachineUtils.h"
#include "Machine.h"
#include "tcb.h"
#include "scheduler.h"	
	
void VMThreadCallback(void*);
volatile TVMTick slp_ctr;
std::vector<TCB*> tb_tcb(1, (TCB*)NULL);
std::priority_queue<TCB*> readyQueue;
std::list<TCB*> sleepList;
unsigned int TCB::inc_tid = 1;
TCB* currentThread;
	
typedef struct{
	TVMThreadEntry entry;
	TCB* tcb;
	void* param;
} SkelParam, *SkelParamRef;

void skel(void* skelparam);
	
/*
 *VMStart
 */

TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[])
{
	MachineInitialize(machinetickms);
	
	slp_ctr = 0;
	MachineRequestAlarm(tickms*1000, VMThreadCallback, NULL);
	MachineEnableSignals();
	TVMMainEntry VMMain;
	
	VMMain = VMLoadModule(argv[0]);
	VMMain(argc,argv);
		
	TMachineSignalState sigstate;
		
    MachineSuspendSignals(&sigstate);
    
    MachineRequestAlarm(0, NULL, NULL);
    
	MachineTerminate(); 
	
	return VM_STATUS_SUCCESS;
}
	
void VMThreadCallback(void* dat)
{
	if(slp_ctr > 0){
		slp_ctr--;
	}
}
	
/*
 *VMThreadSleep
 */

TVMStatus VMThreadSleep(TVMTick tick) 
{
	if(tick == VM_TIMEOUT_INFINITE)
	{
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	} else if(tick == VM_TIMEOUT_IMMEDIATE)
	{
		//YIELD TO THREAD WITH EQUAL PRIORITY
	}
	
	TVMTick term_slp = slp_ctr;
	slp_ctr += tick;
    while(slp_ctr > term_slp);
	
	return VM_STATUS_SUCCESS;
}
	
/*
 *VMThreadCreate
 */
	 
TVMStatus VMThreadCreate(TVMThreadEntry entry, void *param, 
	TVMMemorySize memsize, TVMThreadPriority prio, TVMThreadIDRef tid)
{
	TMachineSignalState sigstate;
	MachineSuspendSignals(&sigstate);
			
	if(entry == NULL)
	{
		MachineResumeSignals(&sigstate);
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
				
	TCB* tcb = new TCB(entry, param, memsize, NULL, prio, tid);

	if(tid == NULL)
	{
		MachineResumeSignals(&sigstate);
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	}
	
	tb_tcb.insert(tb_tcb.begin() + *tid, tcb);
	
	MachineResumeSignals(&sigstate);
		
	return VM_STATUS_SUCCESS;	
}	

/*
 *VMThreadDelete
 */

TVMStatus VMThreadDelete(TVMThreadID thread)
{
	if(tb_tcb.at(thread) == NULL)
	{
		return VM_STATUS_ERROR_INVALID_ID;
	}
	if(tb_tcb.at(thread)->state == VM_THREAD_STATE_DEAD)
	{
		//Delete dead thread
		return VM_STATUS_SUCCESS;
	} else 
	{
		return VM_STATUS_ERROR_INVALID_STATE;
	}
}

/*
 *VMThreadTerminate
 */
 
TVMStatus VMThreadTerminate(TVMThreadID thread)
{
	TCB* tcb = tb_tcb.at(thread);
	tcb->state = VM_THREAD_STATE_DEAD;
	return VM_STATUS_SUCCESS;
}
	
/*
 *VMThreadID
 */

TVMStatus VMThreadID(TVMThreadIDRef threadref)
{
	//puts the thread identifier of the currently running thread in the location
	//specified by threadref
	
	return VM_STATUS_SUCCESS;
}

/*
 *VMThreadState
 */
 
TVMStatus VMThreadState(TVMThreadID thread, TVMThreadStateRef state)
{
	TCB* tcb = tb_tcb.at(thread);
	if(tcb == NULL)
	{
		return VM_STATUS_ERROR_INVALID_ID;
	}
	if(tcb->state != VM_THREAD_STATE_DEAD)
	{
		return VM_STATUS_ERROR_INVALID_STATE;
	}
	*state = tcb->state;
	return VM_STATUS_SUCCESS;
}
 
/*
*VMThreadActivate
*/

TVMStatus VMThreadActivate(TVMThreadID thread)
{
	TMachineSignalState sigstate;
	MachineSuspendSignals(&sigstate);
	
	TCB* tcb = tb_tcb.at(thread);
	//p("In VMThreadActivate. ThreadID: %d", tcb->tid);
	if(tcb == NULL)
	{
		MachineResumeSignals(&sigstate);
		return VM_STATUS_ERROR_INVALID_ID;
	}
	 
	if(tcb->state != VM_THREAD_STATE_DEAD)
	{
		MachineResumeSignals(&sigstate);
		return VM_STATUS_ERROR_INVALID_STATE;
	}
	
	tcb->stackBase = malloc(sizeof(uint8_t)*tcb->memsize);
	
	SkelParam* skelptr = (SkelParam*)malloc(sizeof(SkelParam));
	(*skelptr).entry = tcb->entry;
	(*skelptr).tcb = tcb;
	(*skelptr).param = tcb->param;
		
	MachineContextCreate(tcb->context, skel, skelptr, tcb->stackBase, 
		sizeof(uint8_t)*tcb->memsize);
	
	tcb->state = VM_THREAD_STATE_READY;
	readyQueue.push(tcb);
	
	schedule();
	
	MachineResumeSignals(&sigstate);
	return VM_STATUS_SUCCESS;
}

/*
 *VMMutexCreate
 */

TVMStatus VMMutexCreate(TVMMutexIDRef mutexref)
{
	return VM_STATUS_SUCCESS;
}

/*
 *VMMutexDelete
 */
 
TVMStatus VMMutexDelete(TVMMutexID mutex)
{
	return VM_STATUS_SUCCESS;
} 

/*
 *VMMutexQuery
 */

TVMStatus VMMutexQuery(TVMMutexID mutex, TVMThreadIDRef ownerref)
{
	return VM_STATUS_SUCCESS;
}
	 
/*
 *VMMutexAcquire
 */

TVMStatus VMMutexCreate(TVMMutexID mutex, TVMTick timeout)
{
	return VM_STATUS_SUCCESS;
}
	 
/*
 *VMMutexRelease
 */

TVMStatus VMMutexCreate(TVMMutexID mutex)
{
	return VM_STATUS_SUCCESS;
}
	 
/*
*VMFileWrite
*/
	
void VMFileWrite_handler(void* calldata, int result)
{
	int* status = (int*)calldata;
	*status = result;
}

TVMStatus VMFileWrite(int fd, void *data, int* length)
{
	volatile int status = 0;
	void* dat = (void*) &status;
	MachineFileWrite(fd, data, *length, VMFileWrite_handler, dat);
    while(status == 0);
		
	return VM_STATUS_SUCCESS;
}

/*
 * VMFileOpen
 */
 
TVMStatus VMFileOpen(const char *filename, int flags, int mode, int *filedescriptor)
{
	return VM_STATUS_SUCCESS;
} 

/*
 * VMFileClose
 */
 
TVMStatus VMFileClose(int *filedescriptor)
{
	return VM_STATUS_SUCCESS;
} 

/*
 * VMFileRead
 */
 
TVMStatus VMFileRead(int filedescriptor, void *data, int *length)
{
	return VM_STATUS_SUCCESS;
} 

/*
 * VMFileSeek
 */
 
TVMStatus VMFileSeek(int filedescriptor, int offset, int whence, int *newoffset)
{
	return VM_STATUS_SUCCESS;
} 

void skel(void* skelparam)
{
	SkelParamRef skelptr = (SkelParamRef) skelparam;
	
	TVMThreadEntry entry = skelptr -> entry;
	TCB* tcb = skelptr -> tcb;
	void* param = skelptr -> param;
	
	entry(param);
	VMThreadTerminate(tcb->tid);
	
}
