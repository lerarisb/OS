/* initial */
#include "../h/types.h"
#include "../h/const.h"
#ifndef INITIAL
#define INITIAL

extern int processCount;
extern pcb_t *currentProc;
extern pcb_t *readyQueue;
extern int softBlockCount;
extern int devSemaphore[SEM4DEV];

extern void main();
extern void uTLB_RefillHandler();
extern void genExceptionHandler();

#endif
