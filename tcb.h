#ifndef TCB_H
#define TCB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Machine.h"
#include "VirtualMachine.h"
#include "VirtualMachineUtils.h"

class TCB
{
	public:
		TCB(TVMThreadEntry, void*, TVMMemorySize, void*, TVMThreadPriority, 
			TVMThreadIDRef);
	
		static unsigned int inc_tid;
		TVMThreadID tid;
		TVMThreadState state;
		TVMThreadPriority priority;
		TVMThreadEntry entry;
		void *param;
		TVMMemorySize memsize;
		void* stackBase;
		SMachineContextRef context;
		TVMTick slp_ctr;
		//Mutex Info
		//File Result
};

class ComparePrio
{
	public:
		bool operator()(TCB& tcb1, TCB& tcb2);
};

#ifdef __cplusplus
}
#endif

#endif
