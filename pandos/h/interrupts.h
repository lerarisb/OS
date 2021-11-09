#include "../h/types.h"
#include "../h/const.h"
#ifndef INTERRUPTS
#define INTERRUPTS

extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t *readyQueue;
extern cpu_t start_clock;
extern int semD[SEMNUM];

extern void interruptHandler();
extern void storeState(state_t *blocked, state_t *ready);

#endif
