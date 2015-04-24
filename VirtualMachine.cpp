extern "C"
{
	#include "VirtualMachine.h"
	#include "VirtualMachineUtils.h"
	#include "Machine.h"

	#include <unistd.h>
	#include <stdio.h>
	
	void VMThreadCallback(void*);
	
	volatile TVMTick slp_ctr;
	
	TVMStatus VMStart(int tickms, int machinetickms, int argc, char *argv[])
	{
		MachineInitialize(machinetickms);
		slp_ctr = 0;
		MachineRequestAlarm(tickms*1000, VMThreadCallback, NULL);
		typedef void(*TVMMainEntry)(int argc, char *argv[]);
		TVMMainEntry VMMain;
	
		VMMain = VMLoadModule(argv[0]);
		VMMain(argc,argv);
		
        
        MachineRequestAlarm(0, NULL, NULL);
		return VM_STATUS_SUCCESS;
	}
	
	void VMThreadCallback(void* dat)
	{
		if(slp_ctr > 0)
			slp_ctr--;
	}
	
	TVMStatus VMThreadSleep(TVMTick tick) 
	{
		TVMTick term_slp = slp_ctr;
		slp_ctr += tick;
        while(slp_ctr > term_slp);
		
	}
	
	void VMFileWrite_handler(void* calldata, int result)
	{
		int* status = (int*)calldata;
		*status = result;
//		printf("status: %d\n",*status);
	}

	TVMStatus VMFileWrite(int fd, void *data, int* length)
	{
		volatile int status = 0;
		void* dat = (void*) &status;
		MachineFileWrite(fd, data, *length, VMFileWrite_handler, dat);
        while(status == 0);
		
		//write(fd, data, *length);
		return VM_STATUS_SUCCESS;
	}
}
