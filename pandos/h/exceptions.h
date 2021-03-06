/* exceptions */
#include "../h/types.h"
#include "../h/const.h"
#ifndef EXCEPTIONS
#define EXCEPTIONS

extern void sysHandler();
extern void TLBHandler();
extern void ProgramTrapHandler();
extern void PassUpOrDie(int exception);
extern void terminateProcess(pcb_t *currentProcess);

#endif
