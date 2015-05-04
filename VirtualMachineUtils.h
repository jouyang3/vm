#ifndef VIRTUALMACHINEUTILS_H
#define VIRTUALMACHINEUTILS_H

#include "VirtualMachine.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    TVMMainEntry VMLoadModule(const char *module);
    void VMUnloadModule(void);
    TVMStatus VMFilePrint(int filedescriptor, const char *format, ...);
    
       
#ifdef __cplusplus
}
#endif

#endif
