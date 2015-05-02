
	#include "VirtualMachine.h"
	#include "tcb.h"
	
	#include <stdint.h>
	#include <stdlib.h>
	
	//TVMThreadID tid;
	//TVMThreadState state;
	//TVMThreadPriority priority;
	//TVMThreadEntry VMThreadEntry;
	//void *param;
	//TVMMemorySize memsize;
	//void* stackBase;
	//SMachineContextRef context;
	//TVMTick slp_ctr;
	//Mutex Info
	//File Result
	
	TCB::TCB(TVMThreadEntry entry, void *param, TVMMemorySize memsize, 
		void *stackBase, TVMThreadPriority prio, TVMThreadIDRef tid)
	{
		this->tid = inc_tid++;
		*tid = this->tid;
		state = VM_THREAD_STATE_DEAD;
		priority = prio;
		this->entry = entry;
		this->param = param;
		this->memsize = memsize; 
		this->stackBase = stackBase;
        this->slp_ctr = 0;
        this->quantum = QUANTUM_PER_THREAD;;
		this->context = (SMachineContextRef)malloc(sizeof(SMachineContext));
		
	}	
	
	bool ComparePrio::operator()(TCB& tcb1, TCB& tcb2)
	{
		return tcb1.priority > tcb2.priority;
	}

