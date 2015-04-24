#ifndef VIRTUALMACHINEUTILS_H
#define VIRTUALMACHINEUTILS_H

#include "VirtualMachine.h"

TVMMainEntry VMLoadModule(const char *module);
void VMUnloadModule(void);
TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);


#endif
