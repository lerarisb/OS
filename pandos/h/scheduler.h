/* scheduler */
#include "../h/types.h"
#include "../h/const.h"
#ifndef SCHEDULER
#define SCHEDULER

extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t *readyQueue;

extern void contextSwitch(pcb_PTR p);
extern void scheduler();

#endif