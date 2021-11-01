#include "../h/types.h"
#include "../h/const.h"
#ifndef INTERRUPTS
#define INTERRUPTS

extern void InterruptHandler();
extern void storeState(state_t *blocked, state_t *ready);

#endif
