/* exceptions */
#include "../h/types.h"
#include "../h/const.h"
#ifndef EXCEPTIONS
#define EXCEPTIONS

extern void syscall(int exceptReason);
extern void TLBHandler();
extern void ProgramTrapHandler();
extern void PassUpOrDie(int exception);
extern void terminateProcess(pcb_Ptr *currentProcess);

#endif