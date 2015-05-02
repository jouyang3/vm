#include <map>
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
std::map<int, TCB*> tb_tcb;
std::priority_queue<TCB*> readyQueue;
std::list<TCB*> sleepList;
unsigned int TCB::inc_tid = 0;
TCB* currentThread;
bool done = false;
	
typedef struct{
	TVMThreadEntry entry;
	TCB* tcb;
	void* param;
} SkelParam, *SkelParamRef;

void skel(void* skelparam);
	
/*
 *VMStart
 */

void idle_stub(void* param){ while(1); }

TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[])
{
    TVMMainEntry VMMain;
    VMMain = VMLoadModule(argv[0]);
    
    MachineInitialize(machinetickms);
    MachineRequestAlarm(tickms*1000, VMThreadCallback, NULL);
    MachineEnableSignals();

    //Create IDLE thread if not exist
    	TVMThreadID idle_tid;
    	TCB* idle_tcb = new TCB(idle_stub, NULL, sizeof(uint8_t)*1024, NULL, 0, &idle_tid);
    	idle_tcb->stackBase = malloc(sizeof(uint8_t)*1024);
    	MachineContextCreate(idle_tcb->context, idle_stub, NULL, idle_tcb->stackBase, sizeof(uint8_t)*1024);
    	idle_tcb->state = VM_THREAD_STATE_READY;
	idle_tcb->quantum = QUANTUM_PER_THREAD;
    	readyQueue.push(idle_tcb);
	tb_tcb[0] = idle_tcb;
    
    //Create context for main thread.
    TVMThreadID main_tid;
    TCB* main_tcb = new TCB(NULL, NULL, 0, NULL, VM_THREAD_PRIORITY_LOW, &main_tid);
//    main_tcb->context = NULL;
//    MachineContextCreate(main_tcb->context, NULL, NULL, NULL, 0);
    main_tcb->state = VM_THREAD_STATE_RUNNING;
    currentThread = main_tcb;
    tb_tcb[main_tid] = main_tcb;
	
    VMMain(argc,argv);
    currentThread->state = VM_THREAD_STATE_DEAD;
    //done=true;

    MachineTerminate();
	
    return VM_STATUS_SUCCESS;
}
	
void VMThreadCallback(void* dat)
{
    TMachineSignalState sigstate;
    MachineSuspendSignals(&sigstate);
    p("Callback called.\n");
    for(std::list<TCB*>::iterator it = sleepList.begin(); it != sleepList.end(); it++)
    {
//        p("TCB->tid: %d\n",(*it)->tid);
        TCB* sleepThread = *it;
        sleepThread->slp_ctr++;
        if(sleepThread->slp_ctr >= sleepThread->nikita)
        {
            it = --sleepList.erase(it);
            sleepThread->state = VM_THREAD_STATE_READY;
            readyQueue.push(sleepThread);
        }
    }
    
    if(currentThread->quantum > 0)
        currentThread->quantum--;
    
    schedule();
    MachineResumeSignals(&sigstate);
}
	
/*
 *VMThreadSleep
 */

TVMStatus VMThreadSleep(TVMTick tick) 
{
    p("[VirtualMachine.cpp VMThreadSleep()] tid: %d sleeps for %d.\n", currentThread->tid, tick);
    currentThread->nikita = tick;
    currentThread->state = VM_THREAD_STATE_WAITING;
	if(tick == VM_TIMEOUT_INFINITE)
	{
		return VM_STATUS_ERROR_INVALID_PARAMETER;
	} else if(tick == VM_TIMEOUT_IMMEDIATE)
	{
		//YIELD TO THREAD WITH EQUAL PRIORITY
        schedule();
        return VM_STATUS_SUCCESS;
	}
    

    sleepList.push_back(currentThread);
    schedule();
	
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
    p("[VirtualMachine.cpp VMThreadCreate()] tid: %d\n", *tid);
	tb_tcb[*tid] = tcb;
    
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
	TCB* tcb = tb_tcb[thread];
	tcb->state = VM_THREAD_STATE_DEAD;
    	schedule();
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
	p("[VirtualMachine.cpp VMThreadActivate()] ThreadID: %d", tcb->tid);
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
