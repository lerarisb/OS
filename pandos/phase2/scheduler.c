/* scheduler */


#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/exceptions.h"
#include "../h/scheduler.h"
#include "/usr/include/umps3/umps/libumps.h"

/* Variables used from initial.c */
extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProc;
extern pcb_t*readyQueue;
extern cpu_t startClock;

/*HELPER METHODS */

/*use so you don't have to keep repeating this in other methods */
void contextSwitch(pcb_PTR p){
	currentProc = p;
	LDST(&(p->p_s));
}
/*
void timer(pcb_PTR p, cpu_t time){
	STCK(startClock);
	setTIMER(time);
	contextSwitch(p);
}
*/
/* Uses the round robin algorithm for each process that will be executed */
void scheduler(){

	if(!emptyProcQ(readyQueue)){
		setTIMER(QUANTUM);
		currentProc = removeProcQ(&readyQueue);
		STCK(startClock);
		contextSwitch(currentProc);
	}

	else{
		currentProc = NULL;

		if ((processCount > 0) && (softBlockCount == 0)){
				PANIC();
			}

		else if (processCount ==0){
			HALT();
		}

		else{
			setTIMER(MAXTIME);

			unsigned int setStatus = (ALLOFF | IECON | IMON | TEBITON);
			setSTATUS(setStatus);
			WAIT();
		}
	}
}








