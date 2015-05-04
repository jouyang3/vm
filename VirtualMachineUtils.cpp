#include "VirtualMachineUtils.h"

#include <list>
#include <map>
#include "VirtualMachine.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define SMALL_BUFFER_SIZE 256

std::map<int, TCB*> tb_tcb;
TCB* idleThread;
std::list<TCB*> readyLow;
std::list<TCB*> readyNormal;
std::list<TCB*> readyHigh;
std::list<TCB*> sleepList;

void *VMLibraryHandle = NULL;

TVMMainEntry VMLoadModule(const char *module){
    
    VMLibraryHandle = dlopen(module, RTLD_NOW);
    if(NULL == VMLibraryHandle){
        fprintf(stderr,"Error dlopen failed %s\n",dlerror());
        return NULL;
    }
    
    return (TVMMainEntry)dlsym(VMLibraryHandle, "VMMain");
}

void VMUnloadModule(void){
    if(NULL != VMLibraryHandle){
        dlclose(VMLibraryHandle);
    }
    VMLibraryHandle = NULL;
}

TVMStatus VMFilePrint(int filedescriptor, const char *format, ...){
    va_list ParamList;
    char *OutputBuffer;
    char SmallBuffer[SMALL_BUFFER_SIZE];
    int SizeRequired;
    TVMStatus ReturnValue;
    
    va_start(ParamList, format);
    OutputBuffer = SmallBuffer;
    
    SizeRequired = vsnprintf(OutputBuffer, SMALL_BUFFER_SIZE, format, ParamList);
    if(SizeRequired < SMALL_BUFFER_SIZE){
        ReturnValue = VMFileWrite(filedescriptor, OutputBuffer, &SizeRequired);
        return ReturnValue;
    }
    OutputBuffer = (char *)malloc(sizeof(char) *(SizeRequired + 1));
    SizeRequired = vsnprintf(OutputBuffer, SizeRequired, format, ParamList);
    ReturnValue = VMFileWrite(filedescriptor, OutputBuffer, &SizeRequired);
    free(OutputBuffer);
    return ReturnValue;
}

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
        front = readyLow.front();
        readyLow.pop_front();
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

