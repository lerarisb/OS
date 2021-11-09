/* initial */
#include "../h/types.h"
#include "../h/const.h"
#ifndef INITIAL
#define INITIAL

extern int processCount;
extern pcb_ptr *currentProc;
extern pcb_ptr *readyQueue;
extern int softBlockCount;
extern int DevSemaphore[devINT * DEVPERINT];

extern int main();
extern void uTLB_RefillHandler();
extern void genExceptionHandler();

#endif