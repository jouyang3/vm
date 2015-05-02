#ifndef VIRTUALMACHINEUTILS_H
#define VIRTUALMACHINEUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
//#include <queue>
#include "VirtualMachine.h"


#define p(...) fprintf(stderr, __VA_ARGS__)

//std::priority_queue<TCB*> readyQueue;

TVMMainEntry VMLoadModule(const char *module);
void VMUnloadModule(void);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
