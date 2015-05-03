#include "VirtualMachine.h"

void VMMain(int argc, char *argv[]){
    VMPrint("Going to sleep for 10 ticks\n");
    VMThreadSleep(50);
    VMPrint("Awake\nGoodbye\n");
}

