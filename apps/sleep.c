#include "VirtualMachine.h"

void VMMain(int argc, char *argv[]){
    VMPrint("Going to sleep for 50 ticks\n");
    VMThreadSleep(50);
    VMPrint("Awake\nGoodbye\n");
}

